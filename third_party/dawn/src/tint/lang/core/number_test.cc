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

#include <cmath>
#include <cstdint>
#include <tuple>
#include <variant>
#include <vector>

#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/text/string_stream.h"

#include "gtest/gtest.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core {
namespace {

// Concats any number of std::vectors
template <typename Vec, typename... Vecs>
[[nodiscard]] inline auto Concat(Vec&& v1, Vecs&&... vs) {
    auto total_size = v1.size() + (vs.size() + ...);
    v1.reserve(total_size);
    (std::move(vs.begin(), vs.end(), std::back_inserter(v1)), ...);
    return std::move(v1);
}

// Next ULP up from kHighestF32 for a float64.
constexpr double kHighestF32NextULP = 0x1.fffffe0000001p+127;

// Highest subnormal value for a float32.
constexpr double kHighestF32Subnormal = 0x0.fffffep-126;

// Next ULP up from kHighestF16 for a float64.
constexpr double kHighestF16NextULP = 0x1.ffc0000000001p+15;

// Highest subnormal value for a float16.
constexpr double kHighestF16Subnormal = 0x0.ffcp-14;

constexpr double kLowestF32NextULP = -kHighestF32NextULP;
constexpr double kLowestF16NextULP = -kHighestF16NextULP;

// MSVC (only in release builds) can grumble about some of the inlined numerical overflow /
// underflow that's done in this file. We like to think we know what we're doing, so silence the
// warning.
TINT_BEGIN_DISABLE_WARNING(CONSTANT_OVERFLOW);

TEST(NumberTest, Equality) {
    EXPECT_TRUE(0_a == 0_a);
    EXPECT_TRUE(10_a == 10_a);
    EXPECT_TRUE(-10_a == -10_a);

    EXPECT_TRUE(0_i == 0_i);
    EXPECT_TRUE(10_i == 10_i);
    EXPECT_TRUE(-10_i == -10_i);

    EXPECT_TRUE(0_u == 0_u);
    EXPECT_TRUE(10_u == 10_u);

    EXPECT_TRUE(0._a == 0._a);
    EXPECT_TRUE(0._a == -0._a);
    EXPECT_TRUE(-0._a == 0._a);
    EXPECT_TRUE(-0._a == -0._a);
    EXPECT_TRUE(10._a == 10._a);
    EXPECT_TRUE(-10._a == -10._a);

    EXPECT_TRUE(0_f == 0_f);
    EXPECT_TRUE(0._f == -0._f);
    EXPECT_TRUE(-0._f == 0._f);
    EXPECT_TRUE(-0_f == -0_f);
    EXPECT_TRUE(10_f == 10_f);
    EXPECT_TRUE(-10_f == -10_f);

    EXPECT_TRUE(0_h == 0_h);
    EXPECT_TRUE(0._h == -0._h);
    EXPECT_TRUE(-0._h == 0._h);
    EXPECT_TRUE(-0_h == -0_h);
    EXPECT_TRUE(10_h == 10_h);
    EXPECT_TRUE(-10_h == -10_h);
}

TEST(NumberTest, Inequality) {
    EXPECT_TRUE(0_a != 1_a);
    EXPECT_TRUE(10_a != 11_a);
    EXPECT_TRUE(11_a != 10_a);
    EXPECT_TRUE(-10_a != -11_a);
    EXPECT_TRUE(-11_a != -10_a);

    EXPECT_TRUE(0_i != 1_i);
    EXPECT_TRUE(1_i != 0_i);
    EXPECT_TRUE(10_i != 11_i);
    EXPECT_TRUE(11_i != 10_i);
    EXPECT_TRUE(-10_i != -11_i);
    EXPECT_TRUE(-11_i != -10_i);

    EXPECT_TRUE(0_u != 1_u);
    EXPECT_TRUE(1_u != 0_u);
    EXPECT_TRUE(10_u != 11_u);
    EXPECT_TRUE(11_u != 10_u);

    EXPECT_FALSE(0._a != -0._a);
    EXPECT_FALSE(-0._a != 0._a);
    EXPECT_TRUE(10._a != 11._a);
    EXPECT_TRUE(11._a != 10._a);
    EXPECT_TRUE(-10._a != -11._a);
    EXPECT_TRUE(-11._a != -10._a);

    EXPECT_FALSE(0_f != -0_f);
    EXPECT_FALSE(-0_f != 0_f);
    EXPECT_TRUE(-0_f != -1_f);
    EXPECT_TRUE(-1_f != -0_f);
    EXPECT_TRUE(10_f != -10_f);
    EXPECT_TRUE(-10_f != 10_f);
    EXPECT_TRUE(10_f != 11_f);
    EXPECT_TRUE(-10_f != -11_f);

    EXPECT_FALSE(0_h != -0_h);
    EXPECT_FALSE(-0_h != 0_h);
    EXPECT_TRUE(-0_h != -1_h);
    EXPECT_TRUE(-1_h != -0_h);
    EXPECT_TRUE(10_h != -10_h);
    EXPECT_TRUE(-10_h != 10_h);
    EXPECT_TRUE(10_h != 11_h);
    EXPECT_TRUE(-10_h != -11_h);
}

TEST(NumberTest, CheckedConvertIdentity) {
    EXPECT_EQ(CheckedConvert<AInt>(0_a), 0_a);
    EXPECT_EQ(CheckedConvert<AFloat>(0_a), 0.0_a);
    EXPECT_EQ(CheckedConvert<i32>(0_i), 0_i);
    EXPECT_EQ(CheckedConvert<i8>(i8(0)), i8(0));
    EXPECT_EQ(CheckedConvert<u32>(0_u), 0_u);
    EXPECT_EQ(CheckedConvert<u64>(u64(0)), u64(0));
    EXPECT_EQ(CheckedConvert<u8>(u8(0)), u8(0));
    EXPECT_EQ(CheckedConvert<f32>(0_f), 0_f);
    EXPECT_EQ(CheckedConvert<f16>(0_h), 0_h);

    EXPECT_EQ(CheckedConvert<AInt>(1_a), 1_a);
    EXPECT_EQ(CheckedConvert<AFloat>(1_a), 1.0_a);
    EXPECT_EQ(CheckedConvert<i32>(1_i), 1_i);
    EXPECT_EQ(CheckedConvert<i8>(i8(1)), i8(1));
    EXPECT_EQ(CheckedConvert<u32>(1_u), 1_u);
    EXPECT_EQ(CheckedConvert<u64>(u64(1)), u64(1));
    EXPECT_EQ(CheckedConvert<u8>(u8(1)), u8(1));
    EXPECT_EQ(CheckedConvert<f32>(1_f), 1_f);
    EXPECT_EQ(CheckedConvert<f16>(1_h), 1_h);
}

TEST(NumberTest, CheckedConvertLargestValue) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(i32::Highest())), i32::Highest());
    EXPECT_EQ(CheckedConvert<i8>(AInt(i8::Highest())), i8::Highest());
    EXPECT_EQ(CheckedConvert<u32>(AInt(u32::Highest())), u32::Highest());
    EXPECT_EQ(CheckedConvert<u32>(i32::Highest()), u32(i32::Highest()));
    EXPECT_EQ(CheckedConvert<u64>(AInt::Highest()), u64(AInt::Highest()));
    EXPECT_EQ(CheckedConvert<u8>(AInt(u8::Highest())), u8::Highest());
    EXPECT_EQ(CheckedConvert<f32>(AFloat(f32::Highest())), f32::Highest());
    EXPECT_EQ(CheckedConvert<f16>(AFloat(f16::Highest())), f16::Highest());
}

TEST(NumberTest, CheckedConvertLowestValue) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(i32::Lowest())), i32::Lowest());
    EXPECT_EQ(CheckedConvert<i8>(AInt(i8::Lowest())), i8::Lowest());
    EXPECT_EQ(CheckedConvert<u32>(AInt(u32::Lowest())), u32::Lowest());
    EXPECT_EQ(CheckedConvert<u64>(AInt(u64::Lowest())), u64::Lowest());
    EXPECT_EQ(CheckedConvert<u8>(AInt(u8::Lowest())), u8::Lowest());
    EXPECT_EQ(CheckedConvert<f32>(AFloat(f32::Lowest())), f32::Lowest());
    EXPECT_EQ(CheckedConvert<f16>(AFloat(f16::Lowest())), f16::Lowest());
}

TEST(NumberTest, CheckedConvertSmallestValue) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(0)), i32(0));
    EXPECT_EQ(CheckedConvert<i8>(AInt(0)), i8(0));
    EXPECT_EQ(CheckedConvert<u32>(AInt(0)), u32(0));
    EXPECT_EQ(CheckedConvert<u64>(AInt(0)), u64(0));
    EXPECT_EQ(CheckedConvert<u8>(AInt(0)), u8(0));
    EXPECT_EQ(CheckedConvert<f32>(AFloat(f32::Smallest())), f32::Smallest());
    EXPECT_EQ(CheckedConvert<f16>(AFloat(f16::Smallest())), f16::Smallest());
}

TEST(NumberTest, CheckedConvertExceedsPositiveLimit) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(static_cast<int64_t>(i32::Highest()) + 1)),
              ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<i8>(AInt(static_cast<int64_t>(i8::Highest()) + 1)),
              ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<u32>(AInt(static_cast<uint64_t>(u32::Highest()) + 1)),
              ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<u8>(AInt(static_cast<uint64_t>(u8::Highest()) + 1)),
              ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<i32>(u32::Highest()), ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<i32>(u32(0x80000000)), ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<i8>(u8::Highest()), ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<i8>(u8(0x80)), ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<u32>(f32::Highest()), u32::Highest());
    EXPECT_EQ(CheckedConvert<u8>(f32::Highest()), u8::Highest());
    EXPECT_EQ(CheckedConvert<i32>(f32::Highest()), i32::Highest());
    EXPECT_EQ(CheckedConvert<i8>(f32::Highest()), i8::Highest());
    EXPECT_EQ(CheckedConvert<u32>(AFloat::Highest()), u32::Highest());
    EXPECT_EQ(CheckedConvert<u8>(AFloat::Highest()), u8::Highest());
    EXPECT_EQ(CheckedConvert<i32>(AFloat::Highest()), i32::Highest());
    EXPECT_EQ(CheckedConvert<i8>(AFloat::Highest()), i8::Highest());
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kHighestF32NextULP)),
              ConversionFailure::kExceedsPositiveLimit);
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kHighestF16NextULP)),
              ConversionFailure::kExceedsPositiveLimit);
}

TEST(NumberTest, CheckedConvertExceedsNegativeLimit) {
    EXPECT_EQ(CheckedConvert<i32>(AInt(static_cast<int64_t>(i32::Lowest()) - 1)),
              ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<i8>(AInt(static_cast<int64_t>(i8::Lowest()) - 1)),
              ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u32>(AInt(static_cast<uint64_t>(u32::Lowest()) - 1)),
              ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u64>(AInt(-1)), ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u8>(AInt(static_cast<uint64_t>(u8::Lowest()) - 1)),
              ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u32>(i32(-1)), ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u32>(i32::Lowest()), ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u32>(f32::Lowest()), u32::Lowest());
    EXPECT_EQ(CheckedConvert<u64>(f32::Lowest()), u64::Lowest());
    EXPECT_EQ(CheckedConvert<u8>(i8(-1)), ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u8>(i8::Lowest()), ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<u8>(f32::Lowest()), u8::Lowest());
    EXPECT_EQ(CheckedConvert<i32>(f32::Lowest()), i32::Lowest());
    EXPECT_EQ(CheckedConvert<i8>(f32::Lowest()), i8::Lowest());
    EXPECT_EQ(CheckedConvert<u32>(AFloat::Lowest()), u32::Lowest());
    EXPECT_EQ(CheckedConvert<u8>(AFloat::Lowest()), u8::Lowest());
    EXPECT_EQ(CheckedConvert<i32>(AFloat::Lowest()), i32::Lowest());
    EXPECT_EQ(CheckedConvert<i8>(AFloat::Lowest()), i8::Lowest());
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kLowestF32NextULP)),
              ConversionFailure::kExceedsNegativeLimit);
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kLowestF16NextULP)),
              ConversionFailure::kExceedsNegativeLimit);
}

TEST(NumberTest, CheckedConvertSubnormals) {
    EXPECT_EQ(CheckedConvert<f32>(AFloat(kHighestF32Subnormal)), f32(kHighestF32Subnormal));
    EXPECT_EQ(CheckedConvert<f16>(AFloat(kHighestF16Subnormal)), f16(kHighestF16Subnormal));
    EXPECT_EQ(CheckedConvert<f32>(AFloat(-kHighestF32Subnormal)), f32(-kHighestF32Subnormal));
    EXPECT_EQ(CheckedConvert<f16>(AFloat(-kHighestF16Subnormal)), f16(-kHighestF16Subnormal));
}

// Test cases for f16 subnormal quantization and BitsRepresentation.
// The ULP is based on float rather than double or f16, since F16::Quantize and
// F16::BitsRepresentation take float as input.
constexpr float lowestPositiveNormalF16 = 0x1p-14;
constexpr float lowestPositiveNormalF16PlusULP = 0x1.000002p-14;
constexpr float lowestPositiveNormalF16MinusULP = 0x1.fffffep-15;
constexpr float highestPositiveSubnormalF16 = 0x0.ffcp-14;
constexpr float highestPositiveSubnormalF16PlusULP = 0x1.ff8002p-15;
constexpr float highestPositiveSubnormalF16MinusULP = 0x1.ff7ffep-15;
constexpr float lowestPositiveSubnormalF16 = 0x1.p-24;
constexpr float lowestPositiveSubnormalF16PlusULP = 0x1.000002p-24;
constexpr float lowestPositiveSubnormalF16MinusULP = 0x1.fffffep-25;

constexpr uint16_t lowestPositiveNormalF16Bits = 0x0400u;
constexpr uint16_t highestPositiveSubnormalF16Bits = 0x03ffu;
constexpr uint16_t lowestPositiveSubnormalF16Bits = 0x0001u;

constexpr float highestNegativeNormalF16 = -lowestPositiveNormalF16;
constexpr float highestNegativeNormalF16PlusULP = -lowestPositiveNormalF16MinusULP;
constexpr float highestNegativeNormalF16MinusULP = -lowestPositiveNormalF16PlusULP;
constexpr float lowestNegativeSubnormalF16 = -highestPositiveSubnormalF16;
constexpr float lowestNegativeSubnormalF16PlusULP = -highestPositiveSubnormalF16MinusULP;
constexpr float lowestNegativeSubnormalF16MinusULP = -highestPositiveSubnormalF16PlusULP;
constexpr float highestNegativeSubnormalF16 = -lowestPositiveSubnormalF16;
constexpr float highestNegativeSubnormalF16PlusULP = -lowestPositiveSubnormalF16MinusULP;
constexpr float highestNegativeSubnormalF16MinusULP = -lowestPositiveSubnormalF16PlusULP;

constexpr uint16_t highestNegativeNormalF16Bits = 0x8400u;
constexpr uint16_t lowestNegativeSubnormalF16Bits = 0x83ffu;
constexpr uint16_t highestNegativeSubnormalF16Bits = 0x8001u;

constexpr float f32_nan = std::numeric_limits<float>::quiet_NaN();
constexpr float f32_inf = std::numeric_limits<float>::infinity();

struct F16TestCase {
    float input_value;
    float quantized_value;
    uint16_t f16_bit_pattern;
};

using NumberF16Test = testing::TestWithParam<F16TestCase>;

TEST_P(NumberF16Test, QuantizeF16) {
    float input_value = GetParam().input_value;
    float quantized_value = GetParam().quantized_value;

    StringStream ss;
    ss << "input value = " << input_value << ", expected quantized value = " << quantized_value;
    SCOPED_TRACE(ss.str());

    if (std::isnan(quantized_value)) {
        EXPECT_TRUE(std::isnan(f16(input_value)));
    } else {
        EXPECT_EQ(f16(input_value), quantized_value);
    }
}

TEST_P(NumberF16Test, BitsRepresentation) {
    float input_value = GetParam().input_value;
    uint16_t representation = GetParam().f16_bit_pattern;

    StringStream ss;
    ss << "input value = " << input_value
       << ", expected binary16 bits representation = " << std::hex << std::showbase
       << representation;
    SCOPED_TRACE(ss.str());

    EXPECT_EQ(f16(input_value).BitsRepresentation(), representation);
}

TEST_P(NumberF16Test, FromBits) {
    float input_value = GetParam().quantized_value;
    uint16_t representation = GetParam().f16_bit_pattern;

    StringStream ss;
    ss << "binary16 bits representation = " << std::hex << std::showbase << representation
       << " expected value = " << input_value;
    SCOPED_TRACE(ss.str());

    if (std::isnan(input_value)) {
        EXPECT_TRUE(std::isnan(f16::FromBits(representation)));
    } else {
        EXPECT_EQ(f16::FromBits(representation), f16(input_value));
    }
}

INSTANTIATE_TEST_SUITE_P(
    NumberF16Test,
    NumberF16Test,
    testing::ValuesIn(std::vector<F16TestCase>{
        // NaN, Inf
        {f32_inf, f32_inf, 0x7c00u},
        {-f32_inf, -f32_inf, 0xfc00u},
        {f32_nan, f32_nan, 0x7e00u},
        {-f32_nan, -f32_nan, 0x7e00u},
        // +/- zero
        {+0.0f, 0.0f, 0x0000u},
        {-0.0f, -0.0f, 0x8000u},
        // Value in normal f16 range
        {1.0f, 1.0f, 0x3c00u},
        {-1.0f, -1.0f, 0xbc00u},
        //   0.00006106496 quantized to 0.000061035156 = 0x1p-14
        {0.00006106496f, 0.000061035156f, 0x0400u},
        {-0.00006106496f, -0.000061035156f, 0x8400u},
        //   1.0004883 quantized to 1.0 = 0x1p0
        {1.0004883f, 1.0f, 0x3c00u},
        {-1.0004883f, -1.0f, 0xbc00u},
        //   8196.0 quantized to 8192.0 = 0x1p13
        {8196.0f, 8192.f, 0x7000u},
        {-8196.0f, -8192.f, 0xf000u},
        // Value in subnormal f16 range
        {0x0.034p-14f, 0x0.034p-14f, 0x000du},
        {-0x0.034p-14f, -0x0.034p-14f, 0x800du},
        {0x0.068p-14f, 0x0.068p-14f, 0x001au},
        {-0x0.068p-14f, -0x0.068p-14f, 0x801au},
        //   0x0.06b7p-14 quantized to 0x0.068p-14
        {0x0.06b7p-14f, 0x0.068p-14f, 0x001au},
        {-0x0.06b7p-14f, -0x0.068p-14, 0x801au},
        // Value out of f16 range
        {65504.003f, f32_inf, 0x7c00u},
        {-65504.003f, -f32_inf, 0xfc00u},
        {0x1.234p56f, f32_inf, 0x7c00u},
        {-0x4.321p65f, -f32_inf, 0xfc00u},

        // Test for subnormal quantization.
        // Value larger than or equal to lowest positive normal f16 will be quantized to normal f16.
        {lowestPositiveNormalF16PlusULP, lowestPositiveNormalF16, lowestPositiveNormalF16Bits},
        {lowestPositiveNormalF16, lowestPositiveNormalF16, lowestPositiveNormalF16Bits},
        // Positive value smaller than lowest positive normal f16 but not smaller than lowest
        // positive
        // subnormal f16 will be quantized to subnormal f16 or zero.
        {lowestPositiveNormalF16MinusULP, highestPositiveSubnormalF16,
         highestPositiveSubnormalF16Bits},
        {highestPositiveSubnormalF16PlusULP, highestPositiveSubnormalF16,
         highestPositiveSubnormalF16Bits},
        {highestPositiveSubnormalF16, highestPositiveSubnormalF16, highestPositiveSubnormalF16Bits},
        {highestPositiveSubnormalF16MinusULP, 0x0.ff8p-14, 0x03feu},
        {lowestPositiveSubnormalF16PlusULP, lowestPositiveSubnormalF16,
         lowestPositiveSubnormalF16Bits},
        {lowestPositiveSubnormalF16, lowestPositiveSubnormalF16, lowestPositiveSubnormalF16Bits},
        // Positive value smaller than lowest positive subnormal f16 will be quantized to zero.
        {lowestPositiveSubnormalF16MinusULP, 0.0, 0x0000u},
        // Test the mantissa discarding, the least significant mantissa bit is 0x1p-24 =
        // 0x0.004p-14.
        {0x0.064p-14f, 0x0.064p-14, 0x0019u},
        {0x0.067fecp-14f, 0x0.064p-14, 0x0019u},
        {0x0.063ffep-14f, 0x0.060p-14, 0x0018u},
        {0x0.008p-14f, 0x0.008p-14, 0x0002u},
        {0x0.00bffep-14f, 0x0.008p-14, 0x0002u},
        {0x0.007ffep-14f, 0x0.004p-14, 0x0001u},

        // Vice versa for negative cases.
        {highestNegativeNormalF16MinusULP, highestNegativeNormalF16, highestNegativeNormalF16Bits},
        {highestNegativeNormalF16, highestNegativeNormalF16, highestNegativeNormalF16Bits},
        {highestNegativeNormalF16PlusULP, lowestNegativeSubnormalF16,
         lowestNegativeSubnormalF16Bits},
        {lowestNegativeSubnormalF16MinusULP, lowestNegativeSubnormalF16,
         lowestNegativeSubnormalF16Bits},
        {lowestNegativeSubnormalF16, lowestNegativeSubnormalF16, lowestNegativeSubnormalF16Bits},
        {lowestNegativeSubnormalF16PlusULP, -0x0.ff8p-14, 0x83feu},
        {highestNegativeSubnormalF16MinusULP, highestNegativeSubnormalF16,
         highestNegativeSubnormalF16Bits},
        {highestNegativeSubnormalF16, highestNegativeSubnormalF16, highestNegativeSubnormalF16Bits},
        {highestNegativeSubnormalF16PlusULP, -0.0, 0x8000u},
        // Test the mantissa discarding.
        {-0x0.064p-14f, -0x0.064p-14, 0x8019u},
        {-0x0.067fecp-14f, -0x0.064p-14, 0x8019u},
        {-0x0.063ffep-14f, -0x0.060p-14, 0x8018u},
        {-0x0.008p-14f, -0x0.008p-14, 0x8002u},
        {-0x0.00bffep-14f, -0x0.008p-14, 0x8002u},
        {-0x0.007ffep-14f, -0x0.004p-14, 0x8001u},
        /////////////////////////////////////
    }));

#ifdef OVERFLOW
#undef OVERFLOW  // corecrt_math.h :(
#endif
#define OVERFLOW \
    {}

// An error value.  IEEE 754 exceptions map to this, including overflow,
// invalid operation, and division by zero.
template <typename T>
auto Error = std::optional<T>{};

using BinaryCheckedCase_AInt = std::tuple<std::optional<AInt>, AInt, AInt>;
using CheckedAddTest_AInt = testing::TestWithParam<BinaryCheckedCase_AInt>;

using FloatInputTypes = std::variant<AFloat, f32, f16>;
using FloatExpectedTypes =
    std::variant<std::optional<AFloat>, std::optional<f32>, std::optional<f16>>;
using BinaryCheckedCase_Float = std::tuple<FloatExpectedTypes, FloatInputTypes, FloatInputTypes>;

/// Validates that result is equal to expect. If `float_comp` is true, uses EXPECT_FLOAT_EQ to
/// compare the values.
template <typename T>
void ValidateResult(std::optional<T> result, std::optional<T> expect, bool float_comp = false) {
    if (!expect) {
        EXPECT_TRUE(!result) << *result;
    } else {
        ASSERT_TRUE(result);
        if constexpr (IsIntegral<T>) {
            EXPECT_EQ(*result, *expect);
        } else {
            if (float_comp) {
                EXPECT_FLOAT_EQ(*result, *expect);
            } else {
                EXPECT_EQ(*result, *expect);
            }
        }
    }
}

TEST_P(CheckedAddTest_AInt, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    ValidateResult(CheckedAdd(a, b), expect);
    ValidateResult(CheckedAdd(b, a), expect);
}
INSTANTIATE_TEST_SUITE_P(
    CheckedAddTest_AInt,
    CheckedAddTest_AInt,
    testing::ValuesIn(std::vector<BinaryCheckedCase_AInt>{
        {AInt(0), AInt(0), AInt(0)},
        {AInt(1), AInt(1), AInt(0)},
        {AInt(2), AInt(1), AInt(1)},
        {AInt(0), AInt(-1), AInt(1)},
        {AInt(3), AInt(2), AInt(1)},
        {AInt(-1), AInt(-2), AInt(1)},
        {AInt(0x300), AInt(0x100), AInt(0x200)},
        {AInt(0x100), AInt(-0x100), AInt(0x200)},
        {AInt::Highest(), AInt(1), AInt(AInt::kHighestValue - 1)},
        {AInt::Lowest(), AInt(-1), AInt(AInt::kLowestValue + 1)},
        {AInt::Highest(), AInt(0x7fffffff00000000ll), AInt(0x00000000ffffffffll)},
        {AInt::Highest(), AInt::Highest(), AInt(0)},
        {AInt::Lowest(), AInt::Lowest(), AInt(0)},
        {OVERFLOW, AInt(1), AInt::Highest()},
        {OVERFLOW, AInt(-1), AInt::Lowest()},
        {OVERFLOW, AInt(2), AInt::Highest()},
        {OVERFLOW, AInt(-2), AInt::Lowest()},
        {OVERFLOW, AInt(10000), AInt::Highest()},
        {OVERFLOW, AInt(-10000), AInt::Lowest()},
        {OVERFLOW, AInt::Highest(), AInt::Highest()},
        {OVERFLOW, AInt::Lowest(), AInt::Lowest()},
        ////////////////////////////////////////////////////////////////////////
    }));

using CheckedAddTest_Float = testing::TestWithParam<BinaryCheckedCase_Float>;
TEST_P(CheckedAddTest_Float, Test) {
    auto& p = GetParam();
    std::visit(
        [&](auto&& lhs) {
            using T = std::decay_t<decltype(lhs)>;
            auto rhs = std::get<T>(std::get<2>(p));
            auto expect = std::get<std::optional<T>>(std::get<0>(p));
            EXPECT_TRUE(CheckedAdd(lhs, rhs) == expect)
                << std::hex << "0x" << lhs << " + 0x" << rhs;
            EXPECT_TRUE(CheckedAdd(rhs, lhs) == expect)
                << std::hex << "0x" << lhs << " + 0x" << rhs;
        },
        std::get<1>(p));
}
template <typename T>
std::vector<BinaryCheckedCase_Float> CheckedAddTest_FloatCases() {
    return {
        {T(0), T(0), T(0)},
        {T(1), T(1), T(0)},
        {T(2), T(1), T(1)},
        {T(0), T(-1), T(1)},
        {T(3), T(2), T(1)},
        {T(-1), T(-2), T(1)},
        {T(0x300), T(0x100), T(0x200)},
        {T(0x100), T(-0x100), T(0x200)},
        {T::Highest(), T::Highest(), T(0)},
        {T::Lowest(), T::Lowest(), T(0)},
        {Error<T>, T::Highest(), T::Highest()},
        {Error<T>, T::Lowest(), T::Lowest()},
    };
}
INSTANTIATE_TEST_SUITE_P(CheckedAddTest_Float,
                         CheckedAddTest_Float,
                         testing::ValuesIn(Concat(CheckedAddTest_FloatCases<AFloat>(),
                                                  CheckedAddTest_FloatCases<f32>(),
                                                  CheckedAddTest_FloatCases<f16>())));

using CheckedSubTest_AInt = testing::TestWithParam<BinaryCheckedCase_AInt>;
TEST_P(CheckedSubTest_AInt, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    ValidateResult(CheckedSub(a, b), expect);
}
INSTANTIATE_TEST_SUITE_P(
    CheckedSubTest_AInt,
    CheckedSubTest_AInt,
    testing::ValuesIn(std::vector<BinaryCheckedCase_AInt>{
        {AInt(0), AInt(0), AInt(0)},
        {AInt(1), AInt(1), AInt(0)},
        {AInt(0), AInt(1), AInt(1)},
        {AInt(-2), AInt(-1), AInt(1)},
        {AInt(1), AInt(2), AInt(1)},
        {AInt(-3), AInt(-2), AInt(1)},
        {AInt(0x100), AInt(0x300), AInt(0x200)},
        {AInt(-0x300), AInt(-0x100), AInt(0x200)},
        {AInt::Highest(), AInt(AInt::kHighestValue - 1), AInt(-1)},
        {AInt::Lowest(), AInt(AInt::kLowestValue + 1), AInt(1)},
        {AInt(0x00000000ffffffffll), AInt::Highest(), AInt(0x7fffffff00000000ll)},
        {AInt::Highest(), AInt::Highest(), AInt(0)},
        {AInt::Lowest(), AInt::Lowest(), AInt(0)},
        {OVERFLOW, AInt::Lowest(), AInt(1)},
        {OVERFLOW, AInt::Highest(), AInt(-1)},
        {OVERFLOW, AInt::Lowest(), AInt(2)},
        {OVERFLOW, AInt::Highest(), AInt(-2)},
        {OVERFLOW, AInt::Lowest(), AInt(10000)},
        {OVERFLOW, AInt::Highest(), AInt(-10000)},
        {OVERFLOW, AInt::Lowest(), AInt::Highest()},
        ////////////////////////////////////////////////////////////////////////
    }));

using CheckedSubTest_Float = testing::TestWithParam<BinaryCheckedCase_Float>;
TEST_P(CheckedSubTest_Float, Test) {
    auto& p = GetParam();
    std::visit(
        [&](auto&& lhs) {
            using T = std::decay_t<decltype(lhs)>;
            auto rhs = std::get<T>(std::get<2>(p));
            auto expect = std::get<std::optional<T>>(std::get<0>(p));
            ValidateResult(CheckedSub(lhs, rhs), expect);
        },
        std::get<1>(p));
}
template <typename T>
std::vector<BinaryCheckedCase_Float> CheckedSubTest_FloatCases() {
    return {
        {T(0), T(0), T(0)},
        {T(1), T(1), T(0)},
        {T(0), T(1), T(1)},
        {T(-2), T(-1), T(1)},
        {T(1), T(2), T(1)},
        {T(-3), T(-2), T(1)},
        {T(0x100), T(0x300), T(0x200)},
        {T(-0x300), T(-0x100), T(0x200)},
        {T::Highest(), T::Highest(), T(0)},
        {T::Lowest(), T::Lowest(), T(0)},
        {Error<T>, T::Lowest(), T::Highest()},
    };
}
INSTANTIATE_TEST_SUITE_P(CheckedSubTest_Float,
                         CheckedSubTest_Float,
                         testing::ValuesIn(Concat(CheckedSubTest_FloatCases<AFloat>(),
                                                  CheckedSubTest_FloatCases<f32>(),
                                                  CheckedSubTest_FloatCases<f16>())));

using CheckedMulTest_AInt = testing::TestWithParam<BinaryCheckedCase_AInt>;
TEST_P(CheckedMulTest_AInt, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    ValidateResult(CheckedMul(a, b), expect);
    ValidateResult(CheckedMul(b, a), expect);
}
INSTANTIATE_TEST_SUITE_P(
    CheckedMulTest_AInt,
    CheckedMulTest_AInt,
    testing::ValuesIn(std::vector<BinaryCheckedCase_AInt>{
        {AInt(0), AInt(0), AInt(0)},
        {AInt(0), AInt(1), AInt(0)},
        {AInt(1), AInt(1), AInt(1)},
        {AInt(-1), AInt(-1), AInt(1)},
        {AInt(2), AInt(2), AInt(1)},
        {AInt(-2), AInt(-2), AInt(1)},
        {AInt(0x20000), AInt(0x100), AInt(0x200)},
        {AInt(-0x20000), AInt(-0x100), AInt(0x200)},
        {AInt(0x4000000000000000ll), AInt(0x80000000ll), AInt(0x80000000ll)},
        {AInt(0x4000000000000000ll), AInt(-0x80000000ll), AInt(-0x80000000ll)},
        {AInt(0x1000000000000000ll), AInt(0x40000000ll), AInt(0x40000000ll)},
        {AInt(-0x1000000000000000ll), AInt(-0x40000000ll), AInt(0x40000000ll)},
        {AInt(0x100000000000000ll), AInt(0x1000000), AInt(0x100000000ll)},
        {AInt(0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(2)},
        {AInt(-0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(-2)},
        {AInt(-0x2000000000000000ll), AInt(-0x1000000000000000ll), AInt(2)},
        {AInt(-0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(-2)},
        {AInt(0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(4)},
        {AInt(-0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(-4)},
        {AInt(-0x4000000000000000ll), AInt(-0x1000000000000000ll), AInt(4)},
        {AInt(-0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(-4)},
        {AInt(INT64_MIN), AInt(0x1000000000000000ll), AInt(-8)},
        {AInt(INT64_MIN), AInt(-0x1000000000000000ll), AInt(8)},
        {AInt(0), AInt::Highest(), AInt(0)},
        {AInt(0), AInt::Lowest(), AInt(0)},
        {OVERFLOW, AInt(0x1000000000000000ll), AInt(8)},
        {OVERFLOW, AInt(-0x1000000000000000ll), AInt(-8)},
        {OVERFLOW, AInt(0x800000000000000ll), AInt(0x10)},
        {OVERFLOW, AInt(0x80000000ll), AInt(0x100000000ll)},
        {OVERFLOW, AInt::Highest(), AInt::Highest()},
        {OVERFLOW, AInt::Highest(), AInt::Lowest()},
        ////////////////////////////////////////////////////////////////////////
    }));

using CheckedMulTest_Float = testing::TestWithParam<BinaryCheckedCase_Float>;
TEST_P(CheckedMulTest_Float, Test) {
    auto& p = GetParam();
    std::visit(
        [&](auto&& lhs) {
            using T = std::decay_t<decltype(lhs)>;
            auto rhs = std::get<T>(std::get<2>(p));
            auto expect = std::get<std::optional<T>>(std::get<0>(p));
            ValidateResult(CheckedMul(lhs, rhs), expect);
            ValidateResult(CheckedMul(rhs, lhs), expect);
        },
        std::get<1>(p));
}
template <typename T>
std::vector<BinaryCheckedCase_Float> CheckedMulTest_FloatCases() {
    return {
        {T(0), T(0), T(0)},
        {T(0), T(1), T(0)},
        {T(1), T(1), T(1)},
        {T(-1), T(-1), T(1)},
        {T(2), T(2), T(1)},
        {T(-2), T(-2), T(1)},
        {T(0), T::Highest(), T(0)},
        {T(0), T::Lowest(), -T(0)},
        {Error<T>, T::Highest(), T::Highest()},
        {Error<T>, T::Lowest(), T::Lowest()},
    };
}
INSTANTIATE_TEST_SUITE_P(CheckedMulTest_Float,
                         CheckedMulTest_Float,
                         testing::ValuesIn(Concat(CheckedMulTest_FloatCases<AFloat>(),
                                                  CheckedMulTest_FloatCases<f32>(),
                                                  CheckedMulTest_FloatCases<f16>())));

using CheckedDivTest_AInt = testing::TestWithParam<BinaryCheckedCase_AInt>;
TEST_P(CheckedDivTest_AInt, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    ValidateResult(CheckedDiv(a, b), expect);
}
INSTANTIATE_TEST_SUITE_P(
    CheckedDivTest_AInt,
    CheckedDivTest_AInt,
    testing::ValuesIn(std::vector<BinaryCheckedCase_AInt>{
        {AInt(0), AInt(0), AInt(1)},
        {AInt(1), AInt(1), AInt(1)},
        {AInt(1), AInt(1), AInt(1)},
        {AInt(2), AInt(2), AInt(1)},
        {AInt(2), AInt(4), AInt(2)},
        {AInt::Highest(), AInt::Highest(), AInt(1)},
        {AInt::Lowest(), AInt::Lowest(), AInt(1)},
        {AInt(1), AInt::Highest(), AInt::Highest()},
        {AInt(0), AInt(0), AInt::Highest()},
        {AInt(0), AInt(0), AInt::Lowest()},
        {OVERFLOW, AInt(123), AInt(0)},
        {OVERFLOW, AInt(-123), AInt(0)},
        ////////////////////////////////////////////////////////////////////////
    }));

using CheckedDivTest_Float = testing::TestWithParam<BinaryCheckedCase_Float>;
TEST_P(CheckedDivTest_Float, Test) {
    auto& p = GetParam();
    std::visit(
        [&](auto&& lhs) {
            using T = std::decay_t<decltype(lhs)>;
            auto rhs = std::get<T>(std::get<2>(p));
            auto expect = std::get<std::optional<T>>(std::get<0>(p));
            ValidateResult(CheckedDiv(lhs, rhs), expect);
        },
        std::get<1>(p));
}
template <typename T>
std::vector<BinaryCheckedCase_Float> CheckedDivTest_FloatCases() {
    return {
        {T(0), T(0), T(1)},
        {T(1), T(1), T(1)},
        {T(1), T(1), T(1)},
        {T(2), T(2), T(1)},
        {T(2), T(4), T(2)},
        {T::Highest(), T::Highest(), T(1)},
        {T::Lowest(), T::Lowest(), T(1)},
        {T(1), T::Highest(), T::Highest()},
        {T(0), T(0), T::Highest()},
        {-T(0), T(0), T::Lowest()},
        {Error<T>, T(123), T(0.0)},
        {Error<T>, T(123), T(-0.0)},
        {Error<T>, T(-123), T(0.0)},
        {Error<T>, T(-123), T(-0.0)},
    };
}
INSTANTIATE_TEST_SUITE_P(CheckedDivTest_Float,
                         CheckedDivTest_Float,
                         testing::ValuesIn(Concat(CheckedDivTest_FloatCases<AFloat>(),
                                                  CheckedDivTest_FloatCases<f32>(),
                                                  CheckedDivTest_FloatCases<f16>())));

using CheckedModTest_AInt = testing::TestWithParam<BinaryCheckedCase_AInt>;
TEST_P(CheckedModTest_AInt, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    EXPECT_TRUE(CheckedMod(a, b) == expect) << std::hex << "0x" << a << " % 0x" << b;
}
INSTANTIATE_TEST_SUITE_P(
    CheckedModTest_AInt,
    CheckedModTest_AInt,
    testing::ValuesIn(std::vector<BinaryCheckedCase_AInt>{
        {AInt(0), AInt(0), AInt(1)},
        {AInt(0), AInt(1), AInt(1)},
        {AInt(1), AInt(10), AInt(3)},
        {AInt(2), AInt(10), AInt(4)},
        {AInt(0), AInt::Highest(), AInt::Highest()},
        {AInt(0), AInt::Lowest(), AInt::Lowest()},
        {AInt(2), AInt::Highest(), AInt(5)},
        {AInt(1), AInt::Highest(), AInt(6)},
        {AInt(0), AInt::Highest(), AInt(7)},
        {-AInt{1}, -AInt{10}, AInt{3}},
        {-AInt{2}, -AInt{10}, AInt{4}},
        {AInt{1}, AInt{10}, -AInt{3}},
        {AInt{2}, AInt{10}, -AInt{4}},
        {-AInt{1}, -AInt{10}, -AInt{3}},
        {-AInt{2}, -AInt{10}, -AInt{4}},
        {OVERFLOW, AInt::Highest(), AInt(0)},
        {OVERFLOW, AInt::Lowest(), AInt(0)},
        ////////////////////////////////////////////////////////////////////////
    }));

using CheckedModTest_Float = testing::TestWithParam<BinaryCheckedCase_Float>;
TEST_P(CheckedModTest_Float, Test) {
    auto& p = GetParam();
    std::visit(
        [&](auto&& lhs) {
            using T = std::decay_t<decltype(lhs)>;
            auto rhs = std::get<T>(std::get<2>(p));
            auto expect = std::get<std::optional<T>>(std::get<0>(p));
            ValidateResult(CheckedMod(lhs, rhs), expect);
        },
        std::get<1>(p));
}
template <typename T>
std::vector<BinaryCheckedCase_Float> CheckedModTest_FloatCases() {
    return {
        {T(0.5), T(10.5), T(1)},             //
        {T(0.5), T(10.5), T(2)},             //
        {T(1.5), T(10.5), T(3)},             //
        {T(2.5), T(10.5), T(4)},             //
        {T(0.5), T(10.5), T(5)},             //
        {T(0), T::Highest(), T::Highest()},  //
        {T(0), T::Lowest(), T::Lowest()},    //
        {-T{1}, -T{10}, T{3}},               //
        {-T{2}, -T{10}, T{4}},               //
        {T{1}, T{10}, -T{3}},                //
        {T{2}, T{10}, -T{4}},                //
        {-T{1}, -T{10}, -T{3}},              //
        {-T{2}, -T{10}, -T{4}},              //
        {Error<T>, T(123), T(0.0)},          //
        {Error<T>, T(123), T(-0.0)},         //
        {Error<T>, T(-123), T(0.0)},         //
        {Error<T>, T(-123), T(-0.0)},
    };
}
INSTANTIATE_TEST_SUITE_P(CheckedModTest_Float,
                         CheckedModTest_Float,
                         testing::ValuesIn(Concat(CheckedModTest_FloatCases<AFloat>(),
                                                  CheckedModTest_FloatCases<f32>(),
                                                  CheckedModTest_FloatCases<f16>())));

using CheckedPowTest_Float = testing::TestWithParam<BinaryCheckedCase_Float>;
TEST_P(CheckedPowTest_Float, Test) {
    auto& p = GetParam();
    std::visit(
        [&](auto&& lhs) {
            using T = std::decay_t<decltype(lhs)>;
            auto rhs = std::get<T>(std::get<2>(p));
            auto expect = std::get<std::optional<T>>(std::get<0>(p));
            ValidateResult(CheckedPow(lhs, rhs), expect, /* float_comp */ true);
        },
        std::get<1>(p));
}
template <typename T>
std::vector<BinaryCheckedCase_Float> CheckedPowTest_FloatCases() {
    return {
        {T(0), T(0), T(1)},                     //
        {T(0), T(0), T::Highest()},             //
        {T(1), T(1), T(1)},                     //
        {T(1), T(1), T::Lowest()},              //
        {T(4), T(2), T(2)},                     //
        {T(8), T(2), T(3)},                     //
        {T(1), T(1), T::Highest()},             //
        {T(1), T(1), -T(1)},                    //
        {T(0.25), T(2), -T(2)},                 //
        {T(0.125), T(2), -T(3)},                //
        {T(15.625), T(2.5), T(3)},              //
        {T(11.313708498), T(2), T(3.5)},        //
        {T(24.705294220), T(2.5), T(3.5)},      //
        {T(0.0883883476), T(2), -T(3.5)},       //
        {Error<T>, -T(1), T(1)},                //
        {Error<T>, -T(1), T::Highest()},        //
        {Error<T>, T::Lowest(), T(1)},          //
        {Error<T>, T::Lowest(), T::Highest()},  //
        {Error<T>, T::Lowest(), T::Lowest()},   //
        {Error<T>, T(0), T(0)},                 //
        {Error<T>, T(0), -T(1)},                //
        {Error<T>, T(0), T::Lowest()},          //
        // Check exceptional cases from IEEE 754 `powr` function
        // which has its exceptions derived from the formulation
        // powr(x,y) = exp(y * log(x))
        {Error<T>, T(0.0f), T(-2)},     // (0, less-than-zero)
        {Error<T>, T(-0.0f), T(-2)},    // (0, less-than-zero)
        {Error<T>, T(-2), T(5)},        // (less-than-zero, finite)
        {Error<T>, T(-2), T(0)},        // (less-than-zero, finite)
        {Error<T>, T(-2), T(-5)},       // (less-than-zero, finite)
        {Error<T>, T(0.0f), T(0.0f)},   // (0,0)
        {Error<T>, T(-0.0f), T(0.0f)},  // (0,0)
        {Error<T>, T(0.0f), T(-0.0f)},  // (0,0)
        {Error<T>, T(-0.0), T(-0.0f)},  // (0,0)
    };
}
INSTANTIATE_TEST_SUITE_P(CheckedPowTest_Float,
                         CheckedPowTest_Float,
                         testing::ValuesIn(Concat(CheckedPowTest_FloatCases<AFloat>(),
                                                  CheckedPowTest_FloatCases<f32>(),
                                                  CheckedPowTest_FloatCases<f16>())));

using TernaryCheckedCase = std::tuple<std::optional<AInt>, AInt, AInt, AInt>;

using CheckedMaddTest_AInt = testing::TestWithParam<TernaryCheckedCase>;
TEST_P(CheckedMaddTest_AInt, Test) {
    auto expect = std::get<0>(GetParam());
    auto a = std::get<1>(GetParam());
    auto b = std::get<2>(GetParam());
    auto c = std::get<3>(GetParam());
    ValidateResult(CheckedMadd(a, b, c), expect);
    ValidateResult(CheckedMadd(b, a, c), expect);
}
INSTANTIATE_TEST_SUITE_P(
    CheckedMaddTest_AInt,
    CheckedMaddTest_AInt,
    testing::ValuesIn(std::vector<TernaryCheckedCase>{
        {AInt(0), AInt(0), AInt(0), AInt(0)},
        {AInt(0), AInt(1), AInt(0), AInt(0)},
        {AInt(1), AInt(1), AInt(1), AInt(0)},
        {AInt(2), AInt(1), AInt(1), AInt(1)},
        {AInt(0), AInt(1), AInt(-1), AInt(1)},
        {AInt(-1), AInt(1), AInt(-2), AInt(1)},
        {AInt(-1), AInt(-1), AInt(1), AInt(0)},
        {AInt(2), AInt(2), AInt(1), AInt(0)},
        {AInt(-2), AInt(-2), AInt(1), AInt(0)},
        {AInt(0), AInt::Highest(), AInt(0), AInt(0)},
        {AInt(0), AInt::Lowest(), AInt(0), AInt(0)},
        {AInt(3), AInt(1), AInt(2), AInt(1)},
        {AInt(0x300), AInt(1), AInt(0x100), AInt(0x200)},
        {AInt(0x100), AInt(1), AInt(-0x100), AInt(0x200)},
        {AInt(0x20000), AInt(0x100), AInt(0x200), AInt(0)},
        {AInt(-0x20000), AInt(-0x100), AInt(0x200), AInt(0)},
        {AInt(0x4000000000000000ll), AInt(0x80000000ll), AInt(0x80000000ll), AInt(0)},
        {AInt(0x4000000000000000ll), AInt(-0x80000000ll), AInt(-0x80000000ll), AInt(0)},
        {AInt(0x1000000000000000ll), AInt(0x40000000ll), AInt(0x40000000ll), AInt(0)},
        {AInt(-0x1000000000000000ll), AInt(-0x40000000ll), AInt(0x40000000ll), AInt(0)},
        {AInt(0x100000000000000ll), AInt(0x1000000), AInt(0x100000000ll), AInt(0)},
        {AInt(0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(2), AInt(0)},
        {AInt(-0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(-2), AInt(0)},
        {AInt(-0x2000000000000000ll), AInt(-0x1000000000000000ll), AInt(2), AInt(0)},
        {AInt(-0x2000000000000000ll), AInt(0x1000000000000000ll), AInt(-2), AInt(0)},
        {AInt(0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(4), AInt(0)},
        {AInt(-0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(-4), AInt(0)},
        {AInt(-0x4000000000000000ll), AInt(-0x1000000000000000ll), AInt(4), AInt(0)},
        {AInt(-0x4000000000000000ll), AInt(0x1000000000000000ll), AInt(-4), AInt(0)},
        {AInt(INT64_MIN), AInt(0x1000000000000000ll), AInt(-8), AInt(0)},
        {AInt(INT64_MIN), AInt(-0x1000000000000000ll), AInt(8), AInt(0)},
        {AInt::Highest(), AInt(1), AInt(1), AInt(AInt::kHighestValue - 1)},
        {AInt::Lowest(), AInt(1), AInt(-1), AInt(AInt::kLowestValue + 1)},
        {AInt::Highest(), AInt(1), AInt(0x7fffffff00000000ll), AInt(0x00000000ffffffffll)},
        {AInt::Highest(), AInt(1), AInt::Highest(), AInt(0)},
        {AInt::Lowest(), AInt(1), AInt::Lowest(), AInt(0)},
        {OVERFLOW, AInt(0x1000000000000000ll), AInt(8), AInt(0)},
        {OVERFLOW, AInt(-0x1000000000000000ll), AInt(-8), AInt(0)},
        {OVERFLOW, AInt(0x800000000000000ll), AInt(0x10), AInt(0)},
        {OVERFLOW, AInt(0x80000000ll), AInt(0x100000000ll), AInt(0)},
        {OVERFLOW, AInt::Highest(), AInt::Highest(), AInt(0)},
        {OVERFLOW, AInt::Highest(), AInt::Lowest(), AInt(0)},
        {OVERFLOW, AInt(1), AInt(1), AInt::Highest()},
        {OVERFLOW, AInt(1), AInt(-1), AInt::Lowest()},
        {OVERFLOW, AInt(1), AInt(2), AInt::Highest()},
        {OVERFLOW, AInt(1), AInt(-2), AInt::Lowest()},
        {OVERFLOW, AInt(1), AInt(10000), AInt::Highest()},
        {OVERFLOW, AInt(1), AInt(-10000), AInt::Lowest()},
        {OVERFLOW, AInt(1), AInt::Highest(), AInt::Highest()},
        {OVERFLOW, AInt(1), AInt::Lowest(), AInt::Lowest()},
        {OVERFLOW, AInt(1), AInt::Highest(), AInt(1)},
        {OVERFLOW, AInt(1), AInt::Lowest(), AInt(-1)},
    }));

TINT_END_DISABLE_WARNING(CONSTANT_OVERFLOW);

}  // namespace
}  // namespace tint::core
