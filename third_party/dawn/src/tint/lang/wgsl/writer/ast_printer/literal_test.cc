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

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

#include "gmock/gmock.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::wgsl::writer {
namespace {

// Makes an IEEE 754 binary32 floating point number with
// - 0 sign if sign is 0, 1 otherwise
// - 'exponent_bits' is placed in the exponent space.
//   So, the exponent bias must already be included.
f32 MakeF32(uint32_t sign, uint32_t biased_exponent, uint32_t mantissa) {
    const uint32_t sign_bit = sign ? 0x80000000u : 0u;
    // The binary32 exponent is 8 bits, just below the sign.
    const uint32_t exponent_bits = (biased_exponent & 0xffu) << 23;
    // The mantissa is the bottom 23 bits.
    const uint32_t mantissa_bits = (mantissa & 0x7fffffu);

    uint32_t bits = sign_bit | exponent_bits | mantissa_bits;
    float result = 0.0f;
    static_assert(sizeof(result) == sizeof(bits),
                  "expected float and uint32_t to be the same size");
    std::memcpy(&result, &bits, sizeof(bits));
    return f32(result);
}

// Get the representation of an IEEE 754 binary16 floating point number with
// - 0 sign if sign is 0, 1 otherwise
// - 'exponent_bits' is placed in the exponent space.
// - the exponent bias (15) already be included.
f16 MakeF16(uint32_t sign, uint32_t f16_biased_exponent, uint16_t f16_mantissa) {
    assert((f16_biased_exponent & 0xffffffe0u) == 0);
    assert((f16_mantissa & 0xfc00u) == 0);

    const uint32_t sign_bit = sign ? 0x80000000u : 0u;

    // F16 has a exponent bias of 15, and f32 bias 127. Adding 127-15=112 to the f16-biased exponent
    // to get f32-biased exponent.
    uint32_t f32_biased_exponent = (f16_biased_exponent & 0x1fu) + 112;
    assert((f32_biased_exponent & 0xffffff00u) == 0);

    if (f16_biased_exponent == 0) {
        // +/- zero, or subnormal
        if (f16_mantissa == 0) {
            // +/- zero
            return sign ? f16(-0.0f) : f16(0.0f);
        }
        // Subnormal f16, calc the corresponding exponent and mantissa of normal f32.
        f32_biased_exponent += 1;
        // There must be at least one of the 10 mantissa bits being 1, left-shift the mantissa bits
        // until the most significant 1 bit is left-shifted to 10th bit (count from zero), which
        // will be omitted in the resulting f32 mantissa part.
        assert(f16_mantissa & 0x03ffu);
        while ((f16_mantissa & 0x0400u) == 0) {
            f16_mantissa = static_cast<uint16_t>(f16_mantissa << 1);
            f32_biased_exponent--;
        }
    }

    // The binary32 exponent is 8 bits, just below the sign.
    const uint32_t f32_exponent_bits = (f32_biased_exponent & 0xffu) << 23;
    // The mantissa is the bottom 23 bits.
    const uint32_t f32_mantissa_bits = (f16_mantissa & 0x03ffu) << 13;

    uint32_t bits = sign_bit | f32_exponent_bits | f32_mantissa_bits;
    float result = 0.0f;
    static_assert(sizeof(result) == sizeof(bits),
                  "expected float and uint32_t to be the same size");
    std::memcpy(&result, &bits, sizeof(bits));
    return f16(result);
}

struct F32Data {
    f32 value;
    std::string expected;
};

struct F16Data {
    f16 value;
    std::string expected;
};

inline std::ostream& operator<<(std::ostream& out, F32Data data) {
    out << "{" << data.value << "," << data.expected << "}";
    return out;
}

inline std::ostream& operator<<(std::ostream& out, F16Data data) {
    out << "{" << data.value << "," << data.expected << "}";
    return out;
}

using WgslGenerator_F32LiteralTest = TestParamHelper<F32Data>;

TEST_P(WgslGenerator_F32LiteralTest, Emit) {
    auto* v = Expr(GetParam().value);

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitLiteral(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(Zero,
                         WgslGenerator_F32LiteralTest,
                         ::testing::ValuesIn(std::vector<F32Data>{{0_f, "0.0f"},
                                                                  {MakeF32(0, 0, 0), "0.0f"},
                                                                  {MakeF32(1, 0, 0), "-0.0f"}}));

INSTANTIATE_TEST_SUITE_P(Normal,
                         WgslGenerator_F32LiteralTest,
                         ::testing::ValuesIn(std::vector<F32Data>{{1_f, "1.0f"},
                                                                  {-1_f, "-1.0f"},
                                                                  {101.375_f, "101.375f"}}));

INSTANTIATE_TEST_SUITE_P(Subnormal,
                         WgslGenerator_F32LiteralTest,
                         ::testing::ValuesIn(std::vector<F32Data>{
                             {MakeF32(0, 0, 1), "0x1p-149f"},  // Smallest
                             {MakeF32(1, 0, 1), "-0x1p-149f"},
                             {MakeF32(0, 0, 2), "0x1p-148f"},
                             {MakeF32(1, 0, 2), "-0x1p-148f"},
                             {MakeF32(0, 0, 0x7fffff), "0x1.fffffcp-127f"},   // Largest
                             {MakeF32(1, 0, 0x7fffff), "-0x1.fffffcp-127f"},  // Largest
                             {MakeF32(0, 0, 0xcafebe), "0x1.2bfaf8p-127f"},   // Scattered bits
                             {MakeF32(1, 0, 0xcafebe), "-0x1.2bfaf8p-127f"},  // Scattered bits
                             {MakeF32(0, 0, 0xaaaaa), "0x1.55554p-130f"},     // Scattered bits
                             {MakeF32(1, 0, 0xaaaaa), "-0x1.55554p-130f"},    // Scattered bits
                         }));

INSTANTIATE_TEST_SUITE_P(Infinity,
                         WgslGenerator_F32LiteralTest,
                         ::testing::ValuesIn(std::vector<F32Data>{
                             {MakeF32(0, 255, 0), "0x1p+128f"},
                             {MakeF32(1, 255, 0), "-0x1p+128f"}}));

using WgslGenerator_F16LiteralTest = TestParamHelper<F16Data>;

TEST_P(WgslGenerator_F16LiteralTest, Emit) {
    Enable(wgsl::Extension::kF16);

    auto* v = Expr(GetParam().value);

    ASTPrinter& gen = Build(/* resolve */ false);

    StringStream out;
    gen.EmitLiteral(out, v);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(out.str(), GetParam().expected);
}

INSTANTIATE_TEST_SUITE_P(Zero,
                         WgslGenerator_F16LiteralTest,
                         ::testing::ValuesIn(std::vector<F16Data>{{0_h, "0.0h"},
                                                                  {MakeF16(0, 0, 0), "0.0h"},
                                                                  {MakeF16(1, 0, 0), "-0.0h"}}));

INSTANTIATE_TEST_SUITE_P(Normal,
                         WgslGenerator_F16LiteralTest,
                         ::testing::ValuesIn(std::vector<F16Data>{{1_h, "1.0h"},
                                                                  {-1_h, "-1.0h"},
                                                                  {101.375_h, "101.375h"}}));

INSTANTIATE_TEST_SUITE_P(Subnormal,
                         WgslGenerator_F16LiteralTest,
                         ::testing::ValuesIn(std::vector<F16Data>{
                             {MakeF16(0, 0, 1), "0.00000005960464477539h"},  // Smallest
                             {MakeF16(1, 0, 1), "-0.00000005960464477539h"},
                             {MakeF16(0, 0, 2), "0.00000011920928955078h"},
                             {MakeF16(1, 0, 2), "-0.00000011920928955078h"},
                             {MakeF16(0, 0, 0x3ffu), "0.00006097555160522461h"},   // Largest
                             {MakeF16(1, 0, 0x3ffu), "-0.00006097555160522461h"},  // Largest
                             {MakeF16(0, 0, 0x3afu), "0.00005620718002319336h"},   // Scattered bits
                             {MakeF16(1, 0, 0x3afu), "-0.00005620718002319336h"},  // Scattered bits
                             {MakeF16(0, 0, 0x2c7u), "0.00004237890243530273h"},   // Scattered bits
                             {MakeF16(1, 0, 0x2c7u), "-0.00004237890243530273h"},  // Scattered bits
                         }));

}  // namespace
}  // namespace tint::wgsl::writer
