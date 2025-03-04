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

#include <cstring>

#include "gmock/gmock.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/reader/parser/helper_test.h"

using namespace tint::core::fluent_types;  // NOLINT

namespace tint::wgsl::reader {
namespace {

// Makes an IEEE 754 binary64 floating point number with
// - 0 sign if sign is 0, 1 otherwise
// - 'exponent_bits' is placed in the exponent space.
//   So, the exponent bias must already be included.
double MakeDouble(uint64_t sign, uint64_t biased_exponent, uint64_t mantissa) {
    const uint64_t sign_bit = sign ? 0x8000000000000000u : 0u;
    // The binary64 exponent is 11 bits, just below the sign.
    const uint64_t exponent_bits = (biased_exponent & 0x7FFull) << 52;
    // The mantissa is the bottom 52 bits.
    const uint64_t mantissa_bits = (mantissa & 0xFFFFFFFFFFFFFull);

    uint64_t bits = sign_bit | exponent_bits | mantissa_bits;
    double result = 0.0;
    static_assert(sizeof(result) == sizeof(bits),
                  "expected double and uint64_t to be the same size");
    std::memcpy(&result, &bits, sizeof(bits));
    return result;
}

TEST_F(WGSLParserTest, ConstLiteral_Int) {
    {
        auto p = parser("234");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::IntLiteralExpression>());
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->value, 234);
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->suffix,
                  ast::IntLiteralExpression::Suffix::kNone);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 4u}}));
    }
    {
        auto p = parser("234i");
        auto c = p->const_literal();
        EXPECT_TRUE(c.matched);
        EXPECT_FALSE(c.errored);
        EXPECT_FALSE(p->has_error()) << p->error();
        ASSERT_NE(c.value, nullptr);
        ASSERT_TRUE(c->Is<ast::IntLiteralExpression>());
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->value, 234);
        EXPECT_EQ(c->As<ast::IntLiteralExpression>()->suffix,
                  ast::IntLiteralExpression::Suffix::kI);
        EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
    }
}

TEST_F(WGSLParserTest, ConstLiteral_Uint) {
    auto p = parser("234u");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::IntLiteralExpression>());
    EXPECT_EQ(c->As<ast::IntLiteralExpression>()->value, 234);
    EXPECT_EQ(c->As<ast::IntLiteralExpression>()->suffix, ast::IntLiteralExpression::Suffix::kU);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(WGSLParserTest, ConstLiteral_InvalidFloat_IncompleteExponent) {
    auto p = parser("1.0e+");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), "1:1: incomplete exponent for floating point literal: 1.0e+");
    ASSERT_EQ(c.value, nullptr);
}

struct FloatLiteralTestCase {
    std::string input;
    double expected;
    bool operator==(const FloatLiteralTestCase& other) const {
        return (input == other.input) && std::equal_to<double>()(expected, other.expected);
    }
};

inline std::ostream& operator<<(std::ostream& out, FloatLiteralTestCase data) {
    out << data.input;
    return out;
}

class ParserImplFloatLiteralTest : public WGSLParserTestWithParam<FloatLiteralTestCase> {};
TEST_P(ParserImplFloatLiteralTest, Parse) {
    auto params = GetParam();
    SCOPED_TRACE(params.input);
    auto p = parser(params.input);
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    auto* literal = c->As<ast::FloatLiteralExpression>();
    ASSERT_NE(literal, nullptr);
    // Use EXPECT_EQ instead of EXPECT_DOUBLE_EQ here, because EXPECT_DOUBLE_EQ use AlmostEquals(),
    // which allows an error up to 4 ULPs.
    EXPECT_EQ(literal->value, params.expected)
        << "\n"
        << "got:      " << std::hexfloat << literal->value << "\n"
        << "expected: " << std::hexfloat << params.expected;
    if (params.input.back() == 'f') {
        EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
                  ast::FloatLiteralExpression::Suffix::kF);
    } else if (params.input.back() == 'h') {
        EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
                  ast::FloatLiteralExpression::Suffix::kH);
    } else {
        EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
                  ast::FloatLiteralExpression::Suffix::kNone);
    }
    EXPECT_EQ(c->source.range,
              (Source::Range{{1u, 1u}, {1u, 1u + static_cast<uint32_t>(params.input.size())}}));
}
using FloatLiteralTestCaseList = std::vector<FloatLiteralTestCase>;

INSTANTIATE_TEST_SUITE_P(ParserImplFloatLiteralTest_Float,
                         ParserImplFloatLiteralTest,
                         testing::ValuesIn(FloatLiteralTestCaseList{
                             {"0.0", 0.0},                     // Zero
                             {"1.0", 1.0},                     // One
                             {"1000000000.0", 1e9},            // Billion
                             {"0.0", MakeDouble(0, 0, 0)},     // Zero
                             {"1.0", MakeDouble(0, 1023, 0)},  // One

                             {"234.e12", 234.e12},
                             {"234.e12f", static_cast<double>(234.e12f)},
                             {"234.e2h", static_cast<double>(f16::Quantize(234.e2))},

                             // Tiny cases
                             {"1e-5000", 0.0},
                             {"1e-5000f", 0.0},
                             {"1e-50f", 0.0},
                             {"1e-5000h", 0.0},
                             {"1e-50h", 0.0},
                             {"1e-8h", 0.0},  // The smallest positive subnormal f16 is 5.96e-8

                             // Nearly overflow
                             {"1.e308", 1.e308},
                             {"1.8e307", 1.8e307},
                             {"1.798e307", 1.798e307},
                             {"1.7977e307", 1.7977e307},

                             // Nearly overflow
                             {"1e38f", static_cast<double>(1e38f)},
                             {"4.0e37f", static_cast<double>(4.0e37f)},
                             {"3.5e37f", static_cast<double>(3.5e37f)},
                             {"3.403e37f", static_cast<double>(3.403e37f)},

                             // Nearly overflow
                             {"6e4h", 6e4},
                             {"8.0e3h", 8.0e3},
                             {"3.5e3h", 3.5e3},
                             {"3.403e3h", 3.402e3},  // Quantized
                         }));

const double NegInf = MakeDouble(1, 0x7FF, 0);
const double PosInf = MakeDouble(0, 0x7FF, 0);
FloatLiteralTestCaseList HexFloatCases() {
    return FloatLiteralTestCaseList{
        // Regular numbers
        {"0x0p+0", 0x0p+0},
        {"0x1p+0", 0x1p+0},
        {"0x1p+1", 0x1p+1},
        {"0x1.8p+1", 0x1.8p+1},
        {"0x1.99999ap-4", 0x1.99999ap-4},
        {"0x1p-1", 0x1p-1},
        {"0x1p-2", 0x1p-2},
        {"0x1.8p-1", 0x1.8p-1},
        {"0x0.4p+1", 0x0.4p+1},
        {"0x0.02p+3", 0x0.02p+3},
        {"0x4.4p+1", 0x4.4p+1},
        {"0x8c.02p+3", 0x8c.02p+3},

        // Large numbers
        {"0x1p+9", 0x1p+9},
        {"0x1p+10", 0x1p+10},
        {"0x1.02p+10", 0x1.02p+10},

        // Small numbers
        {"0x1p-9", 0x1p-9},
        {"0x1p-10", 0x1p-10},
        {"0x1.02p-3", 0x1.02p-3},

        // Near lowest non-denorm
        {"0x1p-1020", 0x1p-1020},
        {"0x1p-1021", 0x1p-1021},

        {"0x1p-124f", 0x1p-124},
        {"0x1p-125f", 0x1p-125},

        {"0x1p-12h", 0x1p-12},
        {"0x1p-13h", 0x1p-13},

        // Lowest non-denorm
        {"0x1p-1022", 0x1p-1022},

        {"0x1p-126f", 0x1p-126},

        {"0x1p-14h", 0x1p-14},

        // Denormalized values
        {"0x1p-1023", 0x1p-1023},
        {"0x0.8p-1022", 0x0.8p-1022},
        {"0x1p-1024", 0x1p-1024},
        {"0x0.2p-1021", 0x0.2p-1021},
        {"0x1p-1025", 0x1p-1025},
        {"0x1p-1026", 0x1p-1026},
        {"0x1.8p-1023", 0x1.8p-1023},
        {"0x1.8p-1024", 0x1.8p-1024},

        {"0x1p-127f", 0x1p-127},
        {"0x0.8p-126f", 0x0.8p-126},
        {"0x1p-128f", 0x1p-128},
        {"0x0.2p-125f", 0x0.2p-125},
        {"0x1p-129f", 0x1p-129},
        {"0x1p-130f", 0x1p-130},
        {"0x1.8p-127f", 0x1.8p-127},
        {"0x1.8p-128f", 0x1.8p-128},

        {"0x1p-15h", 0x1p-15},
        {"0x0.8p-14h", 0x0.8p-14},
        {"0x1p-16h", 0x1p-16},
        {"0x0.2p-13h", 0x0.2p-13},
        {"0x1p-17h", 0x1p-17},
        {"0x1p-18h", 0x1p-18},
        {"0x1.8p-15h", 0x1.8p-15},
        {"0x1.8p-16h", 0x1.8p-16},

        // F64 extremities
        {"0x1p-1074", 0x1p-1074},                              // +SmallestDenormal
        {"0x1p-1073", 0x1p-1073},                              // +BiggerDenormal
        {"0x1.ffffffffffffep-1023", 0x1.ffffffffffffep-1023},  // +LargestDenormal
        {"0x0.fffffffffffffp-1022", 0x0.fffffffffffffp-1022},  // +LargestDenormal

        {"0x0.cafebeeff000dp-1022", 0x0.cafebeeff000dp-1022},  // +Subnormal
        {"0x1.2bfaf8p-1052", 0x1.2bfaf8p-1052},                // +Subnormal
        {"0x1.55554p-1055", 0x1.55554p-1055},                  // +Subnormal
        {"0x1.fffffffffffp-1027", 0x1.fffffffffffp-1027},  // +Subnormal, = 0x0.0fffffffffff8p-1022

        // F32 extremities
        {"0x1p-149f", 0x1p-149},                // +SmallestDenormal
        {"0x1p-148f", 0x1p-148},                // +BiggerDenormal
        {"0x1.fffffcp-127f", 0x1.fffffcp-127},  // +LargestDenormal
        {"0x0.fffffep-126f", 0x0.fffffep-126},  // +LargestDenormal
        {"0x1.0p-126f", 0x1.0p-126},            // +SmallestNormal
        {"0x8.0p-129f", 0x8.0p-129},            // +SmallestNormal

        {"0x0.cafebp-129f", 0x0.cafebp-129},    // +Subnormal
        {"0x1.2bfaf8p-127f", 0x1.2bfaf8p-127},  // +Subnormal
        {"0x1.55554p-130f", 0x1.55554p-130},    // +Subnormal

        // F32 exactly representable
        {"0x1.000002p+0f", 0x1.000002p+0},
        {"0x8.0000fp+0f", 0x8.0000fp+0},
        {"0x8.fffffp+0f", 0x8.fffffp+0},
        {"0x8.00003p+0f", 0x8.00003p+0},
        {"0x2.123p+0f", 0x2.123p+0},
        {"0x2.cafefp+0f", 0x2.cafefp+0},
        {"0x0.0000fep-126f", 0x0.0000fep-126},  // Subnormal
        {"0x3.f8p-144f", 0x3.f8p-144},          // Subnormal

        // F16 extremities
        {"0x1p-24h", 0x1p-24},          // +SmallestDenormal
        {"0x1p-23h", 0x1p-23},          // +BiggerDenormal
        {"0x1.ff8p-15h", 0x1.ff8p-15},  // +LargestDenormal
        {"0x0.ffcp-14h", 0x0.ffcp-14},  // +LargestDenormal
        {"0x1.0p-14h", 0x1.0p-14},      // +SmallestNormal
        {"0x8.0p-17h", 0x8.0p-17},      // +SmallestNormal

        {"0x0.a8p-19h", 0x0.a8p-19},  // +Subnormal
        {"0x1.7ap-17h", 0x1.7ap-17},  // +Subnormal
        {"0x1.dp-20h", 0x1.dp-20},    // +Subnormal

        // F16 exactly representable
        {"0x1.004p+0h", 0x1.004p+0},
        {"0x8.02p+0h", 0x8.02p+0},
        {"0x8.fep+0h", 0x8.fep+0},
        {"0x8.06p+0h", 0x8.06p+0},
        {"0x2.128p+0h", 0x2.128p+0},
        {"0x2.ca8p+0h", 0x2.ca8p+0},
        {"0x0.0fcp-14h", 0x0.0fcp-14},  // Subnormal
        {"0x3.f00p-20h", 0x3.f00p-20},  // Subnormal

        // Underflow -> Zero
        {"0x1p-1075", 0.0},  // Exponent underflows
        {"0x1p-5000", 0.0},
        {"0x0.00000000000000000000001p-1022", 0.0},  // Fraction causes underflow
        {"0x0.01p-1073", -0.0},

        {"0x1.0p-9223372036854774784", 0},  // -(INT64_MAX - 1023) (smallest valid exponent)

        // Zero with non-zero exponent -> Zero
        {"0x0p+0", 0.0},
        {"0x0p+1", 0.0},
        {"0x0p-1", 0.0},
        {"0x0p+9999999999", 0.0},
        {"0x0p-9999999999", 0.0},
        // Same, but with very large positive exponents that would cause overflow
        // if the mantissa were non-zero.
        {"0x0p+10000000000000000000", 0.0},     // 10 quintillion   (10,000,000,000,000,000,000)
        {"0x0p+100000000000000000000", 0.0},    // 100 quintillion (100,000,000,000,000,000,000)
        {"0x0.00p+10000000000000000000", 0.0},  // As above 4, but with fractional part
        {"0x0.00p+100000000000000000000", 0.0},
        {"0x0p-10000000000000000000", 0.0},  // As above 8, but with negative exponents
        {"0x0p-100000000000000000000", 0.0},
        {"0x0.00p-10000000000000000000", 0.0},
        {"0x0.00p-100000000000000000000", 0.0},

        // Test parsing
        {"0x0p0", 0.0},
        {"0x0p-0", 0.0},
        {"0x0p+000", 0.0},
        {"0x00000000000000p+000000000000000", 0.0},
        {"0x00000000000000p-000000000000000", 0.0},
        {"0x00000000000001p+000000000000000", 1.0},
        {"0x00000000000001p-000000000000000", 1.0},
        {"0x0000000000000000000001.99999ap-000000000000000004", 0.10000000149011612},
        {"0x2p+0", 2.0},
        {"0xFFp+0", 255.0},
        {"0x0.8p+0", 0.5},
        {"0x0.4p+0", 0.25},
        {"0x0.4p+1", 2 * 0.25},
        {"0x0.4p+2", 4 * 0.25},
        {"0x123Ep+1", 9340.0},
        {"0x1a2b3cP12", 7.024656384e+09},

        // Examples without a binary exponent part.
        {"0x1.", 1.0},
        {"0x.8", 0.5},
        {"0x1.8", 1.5},

        // Examples with a binary exponent and a 'f' suffix.
        {"0x1.p0f", 1.0},
        {"0x.8p2f", 2.0},
        {"0x1.8p-1f", 0.75},
        {"0x2p-2f", 0.5},  // No binary point

        // Examples with a binary exponent and a 'h' suffix.
        {"0x1.p0h", 1.0},
        {"0x.8p2h", 2.0},
        {"0x1.8p-1h", 0.75},
        {"0x2p-2h", 0.5},  // No binary point
    };
}
INSTANTIATE_TEST_SUITE_P(ParserImplFloatLiteralTest_HexFloat,
                         ParserImplFloatLiteralTest,
                         testing::ValuesIn(HexFloatCases()));

// Now test all the same hex float cases, but with 0X instead of 0x
template <typename ARR>
std::vector<FloatLiteralTestCase> UpperCase0X(const ARR& cases) {
    std::vector<FloatLiteralTestCase> result;
    result.reserve(cases.size());
    for (const auto& c : cases) {
        result.emplace_back(c);
        auto& input = result.back().input;
        const auto where = input.find("0x");
        if (where != std::string::npos) {
            input[where + 1] = 'X';
        }
    }
    return result;
}

using UpperCase0XTest = ::testing::Test;
TEST_F(UpperCase0XTest, Samples) {
    const auto cases = FloatLiteralTestCaseList{
        {"absent", 0.0}, {"0x", 1.0}, {"0X", 2.0}, {"  0x1p1", 5.0}, {" examine ", 7.0}};
    const auto expected = FloatLiteralTestCaseList{
        {"absent", 0.0}, {"0X", 1.0}, {"0X", 2.0}, {"  0X1p1", 5.0}, {" examine ", 7.0}};

    auto result = UpperCase0X(cases);
    EXPECT_THAT(result, ::testing::ElementsAreArray(expected));
}

INSTANTIATE_TEST_SUITE_P(ParserImplFloatLiteralTest_HexFloat_UpperCase0X,
                         ParserImplFloatLiteralTest,
                         testing::ValuesIn(UpperCase0X(HexFloatCases())));

// <error, source>
using InvalidLiteralTestCase = std::tuple<const char*, const char*>;

class ParserImplInvalidLiteralTest : public WGSLParserTestWithParam<InvalidLiteralTestCase> {};
TEST_P(ParserImplInvalidLiteralTest, Parse) {
    auto* error = std::get<0>(GetParam());
    auto* source = std::get<1>(GetParam());
    auto p = parser(source);
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_TRUE(c.errored);
    EXPECT_EQ(p->error(), std::string(error));
    ASSERT_EQ(c.value, nullptr);
}

INSTANTIATE_TEST_SUITE_P(
    HexFloatMantissaTooLarge,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: mantissa is too large for hex float"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1.ffffffffffffffff8p0",
                         "0x1f.fffffffffffffff8p0",
                         "0x1ff.ffffffffffffff8p0",
                         "0x1fff.fffffffffffff8p0",
                         "0x1ffff.ffffffffffff8p0",
                         "0x1fffff.fffffffffff8p0",
                         "0x1ffffff.ffffffffff8p0",
                         "0x1fffffff.fffffffff8p0",
                         "0x1ffffffff.ffffffff8p0",
                         "0x1fffffffff.fffffff8p0",
                         "0x1ffffffffff.ffffff8p0",
                         "0x1fffffffffff.fffff8p0",
                         "0x1ffffffffffff.ffff8p0",
                         "0x1fffffffffffff.fff8p0",
                         "0x1ffffffffffffff.ff8p0",
                         "0x1ffffffffffffffff.8p0",
                         "0x1ffffffffffffffff8.p0",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexFloatExponentTooLarge,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: exponent is too large for hex float"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1p+9223372036854774785",
                         "0x1p-9223372036854774785",
                         "0x1p+18446744073709551616",
                         "0x1p-18446744073709551616",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexFloatMissingExponent,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: expected an exponent value for hex float"),
                     testing::ValuesIn(std::vector<const char*>{
                         // Lower case p
                         "0x0p",
                         "0x0p+",
                         "0x0p-",
                         "0x1.0p",
                         "0x0.1p",
                         // Upper case p
                         "0x0P",
                         "0x0P+",
                         "0x0P-",
                         "0x1.0P",
                         "0x0.1P",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexNaNAFloat,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'abstract-float'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1.8p+1024",
                         "0x1.0002p+1024",
                         "0x1.0018p+1024",
                         "0x1.01ep+1024",
                         "0x1.fffffep+1024",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexNaNF32,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'f32'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1.8p+128f",
                         "0x1.0002p+128f",
                         "0x1.0018p+128f",
                         "0x1.01ep+128f",
                         "0x1.fffffep+128f",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexNaNF16,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'f16'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1.8p+16h",
                         "0x1.004p+16h",
                         "0x1.018p+16h",
                         "0x1.1ep+16h",
                         "0x1.ffcp+16h",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexOverflowAFloat,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'abstract-float'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1p+1024",
                         "0x1.1p+1024",
                         "0x1p+1025",
                         "0x32p+1023",
                         "0x32p+5000",
                         "0x1.0p9223372036854774784",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexOverflowF32,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'f32'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1p+128f",
                         "0x1.1p+128f",
                         "0x1p+129f",
                         "0x32p+127f",
                         "0x32p+500f",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexOverflowF16,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'f16'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1p+16h",
                         "0x1.1p+16h",
                         "0x1p+17h",
                         "0x32p+15h",
                         "0x32p+500h",
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexNotExactlyRepresentableF32,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be exactly represented as 'f32'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "0x1.000001p+0f",           // Quantizes to 0x1.0p+0
                         "0x1.0000008p+0f",          // Quantizes to 0x1.0p+0
                         "0x1.0000000000001p+0f",    // Quantizes to 0x1.0p+0
                         "0x8.0000f8p+0f",           // Quantizes to 0x8.0000fp+0
                         "0x8.000038p+0f",           // Quantizes to 0x8.00003p+0
                         "0x2.cafef00dp+0f",         // Quantizes to 0x2.cafefp+0
                         "0x0.0000ffp-126f",         // Subnormal, quantizes to 0x0.0000fep-126
                         "0x3.fcp-144f",             // Subnormal, quantizes to 0x3.f8p-144
                         "0x0.ffffffp-126f",         // Subnormal, quantizes to 0x0.fffffep-144
                         "0x0.fffffe0000001p-126f",  // Subnormal, quantizes to 0x0.fffffep-144
                         "0x1.8p-149f",              // Subnormal, quantizes to 0x1.0p-149f
                         "0x1.4p-149f",              // Subnormal, quantizes to 0x1.0p-149f
                         "0x1.000002p-149f",         // Subnormal, quantizes to 0x1.0p-149f
                         "0x1.0000000000001p-149f",  // Subnormal, quantizes to 0x1.0p-149f
                         "0x1.0p-150f",  // Smaller than the smallest subnormal, quantizes to 0.0
                         "0x1.8p-150f",  // Smaller than the smallest subnormal, quantizes to 0.0
                     })));

INSTANTIATE_TEST_SUITE_P(
    HexNotExactlyRepresentableF16,
    ParserImplInvalidLiteralTest,
    testing::Combine(
        testing::Values("1:1: value cannot be exactly represented as 'f16'"),
        testing::ValuesIn(std::vector<const char*>{
            "0x1.002p+0h",             // Quantizes to 0x1.0p+0, has 11 mantissa bits rather than 10
            "0x1.001p+0h",             // Quantizes to 0x1.0p+0, has 12 mantissa bits rather than 10
            "0x1.0000000000001p+0h",   // Quantizes to 0x1.0p+0, has 52 mantissa bits rather than 10
            "0x8.0fp+0h",              // Quantizes to 0x8.0ep+0
            "0x8.31p+0h",              // Quantizes to 0x8.30p+0
            "0x2.ca80dp+0h",           // Quantizes to 0x2.ca8p+0
            "0x4.ba8p+0h",             // Quantizes to 0x4.bap+0
            "0x4.011p+0h",             // Quantizes to 0x4.01p+0
            "0x0.0fep-14h",            // Subnormal, quantizes to 0x0.0fcp-14
            "0x3.f8p-20h",             // Subnormal, quantizes to 0x3.f0p-20
            "0x0.ffep-14h",            // Subnormal, quantizes to 0x0.ffcp-14
            "0x0.ffe0000000001p-14h",  // Subnormal, quantizes to 0x0.ffcp-14
            "0x0.fffffffffffffp-14h",  // Subnormal, quantizes to 0x0.ffcp-14
            "0x1.8p-24h",              // Subnormal, quantizes to 0x1.0p-24f
            "0x1.4p-24h",              // Subnormal, quantizes to 0x1.0p-24f
            "0x1.004p-24h",            // Subnormal, quantizes to 0x1.0p-24f
            "0x1.0000000000001p-24h",  // Subnormal, quantizes to 0x1.0p-24f
            "0x1.0p-25h",              // Smaller than the smallest subnormal, quantizes to 0.0
            "0x1.8p-25h",              // Smaller than the smallest subnormal, quantizes to 0.0
        })));

INSTANTIATE_TEST_SUITE_P(
    DecOverflowAFloat,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'abstract-float'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "1.e309",
                         "1.8e308",
                         "1.798e308",
                         "1.7977e308",
                         "1.2e+5000",
                     })));

INSTANTIATE_TEST_SUITE_P(
    DecOverflowF32,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'f32'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "1e39f",
                         "4.0e38f",
                         "3.5e38f",
                         "3.403e38f",
                         "1.2e+256f",
                     })));

INSTANTIATE_TEST_SUITE_P(
    DecOverflowF16,
    ParserImplInvalidLiteralTest,
    testing::Combine(testing::Values("1:1: value cannot be represented as 'f16'"),
                     testing::ValuesIn(std::vector<const char*>{
                         "1.0e5h",
                         "7.0e4h",
                         "6.6e4h",
                         "6.56e4h",
                         "6.554e4h",
                         "1.2e+32h",
                     })));

TEST_F(WGSLParserTest, ConstLiteral_FloatHighest) {
    const auto highest = std::numeric_limits<float>::max();
    const auto expected_highest = 340282346638528859811704183484516925440.0f;
    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }
    auto p = parser("340282346638528859811704183484516925440.0");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::FloatLiteralExpression>());
    EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->value, std::numeric_limits<float>::max());
    EXPECT_EQ(c->As<ast::FloatLiteralExpression>()->suffix,
              ast::FloatLiteralExpression::Suffix::kNone);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 42u}}));
}

TEST_F(WGSLParserTest, ConstLiteral_True) {
    auto p = parser("true");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::BoolLiteralExpression>());
    EXPECT_TRUE(c->As<ast::BoolLiteralExpression>()->value);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(WGSLParserTest, ConstLiteral_False) {
    auto p = parser("false");
    auto c = p->const_literal();
    EXPECT_TRUE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(c.value, nullptr);
    ASSERT_TRUE(c->Is<ast::BoolLiteralExpression>());
    EXPECT_FALSE(c->As<ast::BoolLiteralExpression>()->value);
    EXPECT_EQ(c->source.range, (Source::Range{{1u, 1u}, {1u, 6u}}));
}

TEST_F(WGSLParserTest, ConstLiteral_NoMatch) {
    auto p = parser("another-token");
    auto c = p->const_literal();
    EXPECT_FALSE(c.matched);
    EXPECT_FALSE(c.errored);
    EXPECT_FALSE(p->has_error()) << p->error();
    ASSERT_EQ(c.value, nullptr);
}

}  // namespace
}  // namespace tint::wgsl::reader
