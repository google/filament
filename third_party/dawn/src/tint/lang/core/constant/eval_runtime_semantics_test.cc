// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/core/constant/eval_test.h"

#include "src/tint/lang/core/constant/scalar.h"

using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::core::constant::test {
namespace {

class ConstEvalRuntimeSemanticsTest : public ConstEvalTest {
  protected:
    /// Default constructor.
    ConstEvalRuntimeSemanticsTest()
        : eval(Eval(constants, Diagnostics(), /* use_runtime_semantics */ true)) {}

    /// The Eval object used during testing (has runtime semantics enabled).
    Eval eval;

    /// @returns the contents of the diagnostics list as a string
    std::string error() { return Diagnostics().Str(); }
};

TEST_F(ConstEvalRuntimeSemanticsTest, Add_AInt_Overflow) {
    auto* a = constants.Get(AInt::Highest());
    auto* b = constants.Get(AInt(1));
    auto result = eval.Plus(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 0);
    EXPECT_EQ(error(),
              R"(warning: '9223372036854775807 + 1' cannot be represented as 'abstract-int')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Add_AFloat_Overflow) {
    auto* a = constants.Get(AFloat::Highest());
    auto* b = constants.Get(AFloat::Highest());
    auto result = eval.Plus(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 + 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0' cannot be represented as 'abstract-float')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Add_F32_Overflow) {
    auto* a = constants.Get(f32::Highest());
    auto* b = constants.Get(f32::Highest());
    auto result = eval.Plus(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '340282346638528859811704183484516925440.0 + 340282346638528859811704183484516925440.0' cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Sub_AInt_Overflow) {
    auto* a = constants.Get(AInt::Lowest());
    auto* b = constants.Get(AInt(1));
    auto result = eval.Minus(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 0);
    EXPECT_EQ(error(),
              R"(warning: '-9223372036854775808 - 1' cannot be represented as 'abstract-int')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Sub_AFloat_Overflow) {
    auto* a = constants.Get(AFloat::Lowest());
    auto* b = constants.Get(AFloat::Highest());
    auto result = eval.Minus(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '-179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 - 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0' cannot be represented as 'abstract-float')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Sub_F32_Overflow) {
    auto* a = constants.Get(f32::Lowest());
    auto* b = constants.Get(f32::Highest());
    auto result = eval.Minus(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '-340282346638528859811704183484516925440.0 - 340282346638528859811704183484516925440.0' cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mul_AInt_Overflow) {
    auto* a = constants.Get(AInt::Highest());
    auto* b = constants.Get(AInt(2));
    auto result = eval.Multiply(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 0);
    EXPECT_EQ(error(),
              R"(warning: '9223372036854775807 * 2' cannot be represented as 'abstract-int')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mul_AFloat_Overflow) {
    auto* a = constants.Get(AFloat::Highest());
    auto* b = constants.Get(AFloat::Highest());
    auto result = eval.Multiply(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 * 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0' cannot be represented as 'abstract-float')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mul_F32_Overflow) {
    auto* a = constants.Get(f32::Highest());
    auto* b = constants.Get(f32::Highest());
    auto result = eval.Multiply(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(
        error(),
        R"(warning: '340282346638528859811704183484516925440.0 * 340282346638528859811704183484516925440.0' cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Div_AInt_ZeroDenominator) {
    auto* a = constants.Get(AInt(42));
    auto* b = constants.Get(AInt(0));
    auto result = eval.Divide(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 42);
    EXPECT_EQ(error(), R"(warning: '42 / 0' cannot be represented as 'abstract-int')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Div_I32_ZeroDenominator) {
    auto* a = constants.Get(i32(42));
    auto* b = constants.Get(i32(0));
    auto result = eval.Divide(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 42);
    EXPECT_EQ(error(), R"(warning: '42 / 0' cannot be represented as 'i32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Div_U32_ZeroDenominator) {
    auto* a = constants.Get(u32(42));
    auto* b = constants.Get(u32(0));
    auto result = eval.Divide(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 42);
    EXPECT_EQ(error(), R"(warning: '42 / 0' cannot be represented as 'u32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Div_AFloat_ZeroDenominator) {
    auto* a = constants.Get(AFloat(42));
    auto* b = constants.Get(AFloat(0));
    auto result = eval.Divide(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 42.f);
    EXPECT_EQ(error(), R"(warning: '42.0 / 0.0' cannot be represented as 'abstract-float')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Div_F32_ZeroDenominator) {
    auto* a = constants.Get(f32(42));
    auto* b = constants.Get(f32(0));
    auto result = eval.Divide(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 42.f);
    EXPECT_EQ(error(), R"(warning: '42.0 / 0.0' cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Div_I32_MostNegativeByMinInt) {
    auto* a = constants.Get(i32::Lowest());
    auto* b = constants.Get(i32(-1));
    auto result = eval.Divide(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), i32::Lowest());
    EXPECT_EQ(error(), R"(warning: '-2147483648 / -1' cannot be represented as 'i32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mod_AInt_ZeroDenominator) {
    auto* a = constants.Get(AInt(42));
    auto* b = constants.Get(AInt(0));
    auto result = eval.Modulo(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), 0);
    EXPECT_EQ(error(), R"(warning: '42 % 0' cannot be represented as 'abstract-int')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mod_I32_ZeroDenominator) {
    auto* a = constants.Get(i32(42));
    auto* b = constants.Get(i32(0));
    auto result = eval.Modulo(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 0);
    EXPECT_EQ(error(), R"(warning: '42 % 0' cannot be represented as 'i32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mod_U32_ZeroDenominator) {
    auto* a = constants.Get(u32(42));
    auto* b = constants.Get(u32(0));
    auto result = eval.Modulo(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0);
    EXPECT_EQ(error(), R"(warning: '42 % 0' cannot be represented as 'u32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mod_AFloat_ZeroDenominator) {
    auto* a = constants.Get(AFloat(42));
    auto* b = constants.Get(AFloat(0));
    auto result = eval.Modulo(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AFloat>(), 0.f);
    EXPECT_EQ(error(), R"(warning: '42.0 % 0.0' cannot be represented as 'abstract-float')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mod_F32_ZeroDenominator) {
    auto* a = constants.Get(f32(42));
    auto* b = constants.Get(f32(0));
    auto result = eval.Modulo(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: '42.0 % 0.0' cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Mod_I32_MostNegativeByMinInt) {
    auto* a = constants.Get(i32::Lowest());
    auto* b = constants.Get(i32(-1));
    auto result = eval.Modulo(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 0);
    EXPECT_EQ(error(), R"(warning: '-2147483648 % -1' cannot be represented as 'i32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, ShiftLeft_AInt_SignChange) {
    auto* a = constants.Get(AInt(0x0FFFFFFFFFFFFFFFll));
    auto* b = constants.Get(u32(9));
    auto result = eval.ShiftLeft(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<AInt>(), static_cast<AInt>(0x0FFFFFFFFFFFFFFFull << 9));
    EXPECT_EQ(error(), R"(warning: shift left operation results in sign change)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, ShiftLeft_I32_SignChange) {
    auto* a = constants.Get(i32(0x0FFFFFFF));
    auto* b = constants.Get(u32(9));
    auto result = eval.ShiftLeft(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), static_cast<i32>(0x0FFFFFFFu << 9));
    EXPECT_EQ(error(), R"(warning: shift left operation results in sign change)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, ShiftLeft_I32_MoreThanBitWidth) {
    auto* a = constants.Get(i32(0x1));
    auto* b = constants.Get(u32(33));
    auto result = eval.ShiftLeft(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 2);
    EXPECT_EQ(
        error(),
        R"(warning: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, ShiftLeft_U32_MoreThanBitWidth) {
    auto* a = constants.Get(u32(0x1));
    auto* b = constants.Get(u32(33));
    auto result = eval.ShiftLeft(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 2);
    EXPECT_EQ(
        error(),
        R"(warning: shift left value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, ShiftRight_I32_MoreThanBitWidth) {
    auto* a = constants.Get(i32(0x2));
    auto* b = constants.Get(u32(33));
    auto result = eval.ShiftRight(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 1);
    EXPECT_EQ(
        error(),
        R"(warning: shift right value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, ShiftRight_U32_MoreThanBitWidth) {
    auto* a = constants.Get(u32(0x2));
    auto* b = constants.Get(u32(33));
    auto result = eval.ShiftRight(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 1);
    EXPECT_EQ(
        error(),
        R"(warning: shift right value must be less than the bit width of the lhs, which is 32)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Acos_F32_OutOfRange) {
    auto* a = constants.Get(f32(2));
    auto result = eval.acos(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(),
              R"(warning: acos must be called with a value in the range [-1 .. 1] (inclusive))");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Acosh_F32_OutOfRange) {
    auto* a = constants.Get(f32(-1));
    auto result = eval.acosh(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: acosh must be called with a value >= 1.0)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Asin_F32_OutOfRange) {
    auto* a = constants.Get(f32(2));
    auto result = eval.asin(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(),
              R"(warning: asin must be called with a value in the range [-1 .. 1] (inclusive))");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Atanh_F32_OutOfRange) {
    auto* a = constants.Get(f32(2));
    auto result = eval.atanh(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(),
              R"(warning: atanh must be called with a value in the range (-1 .. 1) (exclusive))");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Exp_F32_Overflow) {
    auto* a = constants.Get(f32(1000));
    auto result = eval.exp(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: e^1000.0 cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Exp2_F32_Overflow) {
    auto* a = constants.Get(f32(1000));
    auto result = eval.exp2(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: 2^1000.0 cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, ExtractBits_I32_TooManyBits) {
    auto* a = constants.Get(i32(0x12345678));
    auto* offset = constants.Get(u32(24));
    auto* count = constants.Get(u32(16));
    auto result = eval.extractBits(a->Type(), Vector{a, offset, count}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 0x12);
    EXPECT_EQ(error(),
              R"(warning: 'offset' + 'count' must be less than or equal to the bit width of 'e')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, ExtractBits_U32_TooManyBits) {
    auto* a = constants.Get(u32(0x12345678));
    auto* offset = constants.Get(u32(24));
    auto* count = constants.Get(u32(16));
    auto result = eval.extractBits(a->Type(), Vector{a, offset, count}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0x12);
    EXPECT_EQ(error(),
              R"(warning: 'offset' + 'count' must be less than or equal to the bit width of 'e')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, InsertBits_I32_TooManyBits) {
    auto* a = constants.Get(i32(0x99345678));
    auto* b = constants.Get(i32(0x12));
    auto* offset = constants.Get(u32(24));
    auto* count = constants.Get(u32(16));
    auto result = eval.insertBits(a->Type(), Vector{a, b, offset, count}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<i32>(), 0x12345678);
    EXPECT_EQ(error(),
              R"(warning: 'offset' + 'count' must be less than or equal to the bit width of 'e')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, InsertBits_U32_TooManyBits) {
    auto* a = constants.Get(u32(0x99345678));
    auto* b = constants.Get(u32(0x12));
    auto* offset = constants.Get(u32(24));
    auto* count = constants.Get(u32(16));
    auto result = eval.insertBits(a->Type(), Vector{a, b, offset, count}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0x12345678);
    EXPECT_EQ(error(),
              R"(warning: 'offset' + 'count' must be less than or equal to the bit width of 'e')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, InverseSqrt_F32_OutOfRange) {
    auto* a = constants.Get(f32(-1));
    auto result = eval.inverseSqrt(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: inverseSqrt must be called with a value > 0)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, LDExpr_F32_OutOfRange) {
    auto* a = constants.Get(f32(42.f));
    auto* b = constants.Get(f32(200));
    auto result = eval.ldexp(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: e2 must be less than or equal to 128)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Log_F32_OutOfRange) {
    auto* a = constants.Get(f32(-1));
    auto result = eval.log(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: log must be called with a value > 0)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Log2_F32_OutOfRange) {
    auto* a = constants.Get(f32(-1));
    auto result = eval.log2(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: log2 must be called with a value > 0)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Normalize_ZeroLength) {
    auto* zero = constants.Get(f32(0));
    auto* vec =
        eval.VecSplat(create<core::type::Vector>(create<core::type::F32>(), 4u), Vector{zero}, {})
            .Get();
    auto result = eval.normalize(vec->Type(), Vector{vec}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->Index(0)->ValueAs<f32>(), 0.f);
    EXPECT_EQ(result.Get()->Index(1)->ValueAs<f32>(), 0.f);
    EXPECT_EQ(result.Get()->Index(2)->ValueAs<f32>(), 0.f);
    EXPECT_EQ(result.Get()->Index(3)->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: zero length vector can not be normalized)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Pack2x16Float_OutOfRange) {
    auto* a = constants.Get(f32(75250.f));
    auto* b = constants.Get(f32(42.1f));
    auto* vec =
        eval.VecInitS(create<core::type::Vector>(create<core::type::F32>(), 2u), Vector{a, b}, {})
            .Get();
    auto result = eval.pack2x16float(create<core::type::U32>(), Vector{vec}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0x51430000);
    EXPECT_EQ(error(), R"(warning: value 75250.0 cannot be represented as 'f16')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Pow_F32_Overflow) {
    auto* a = constants.Get(f32(2));
    auto* b = constants.Get(f32(1000));
    auto result = eval.pow(a->Type(), Vector{a, b}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: '2.0 ^ 1000.0' cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Unpack2x16Float_OutOfRange) {
    auto* vec2f = create<core::type::Vector>(create<core::type::F32>(), 2u);
    auto* a = constants.Get(u32(0x51437C00));
    auto result = eval.unpack2x16float(vec2f, Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_FLOAT_EQ(result.Get()->Index(0)->ValueAs<f32>(), 0.f);
    EXPECT_FLOAT_EQ(result.Get()->Index(1)->ValueAs<f32>(), 42.09375f);
    EXPECT_EQ(error(), R"(warning: value inf cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, QuantizeToF16_OutOfRange) {
    auto* a = constants.Get(f32(75250.f));
    auto result = eval.quantizeToF16(create<core::type::U32>(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<u32>(), 0);
    EXPECT_EQ(error(), R"(warning: value 75250.0 cannot be represented as 'f16')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Sqrt_F32_OutOfRange) {
    auto* a = constants.Get(f32(-1));
    auto result = eval.sqrt(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: sqrt must be called with a value >= 0)");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Clamp_F32_LowGreaterThanHigh) {
    auto* e = constants.Get(f32(-1));
    auto* low = constants.Get(f32(2));
    auto* high = constants.Get(f32(1));
    auto result = eval.clamp(e->Type(), Vector{e, low, high}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 1.f);
    EXPECT_EQ(error(), R"(warning: clamp called with 'low' (2.0) greater than 'high' (1.0))");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Bitcast_Infinity) {
    auto* a = constants.Get(u32(0x7F800000));
    auto result = eval.bitcast(create<core::type::F32>(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: value inf cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Bitcast_NaN) {
    auto* a = constants.Get(u32(0x7FC00000));
    auto result = eval.bitcast(create<core::type::F32>(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), 0.f);
    EXPECT_EQ(error(), R"(warning: value nan cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Convert_F32_TooHigh) {
    auto* a = constants.Get(AFloat::Highest());
    auto result = eval.Convert(create<core::type::F32>(), a, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), f32::kHighestValue);
    EXPECT_EQ(
        error(),
        R"(warning: value 179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Convert_F32_TooLow) {
    auto* a = constants.Get(AFloat::Lowest());
    auto result = eval.Convert(create<core::type::F32>(), a, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), f32::kLowestValue);
    EXPECT_EQ(
        error(),
        R"(warning: value -179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0 cannot be represented as 'f32')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Convert_F16_TooHigh) {
    auto* a = constants.Get(f32(1000000.0));
    auto result = eval.Convert(create<core::type::F16>(), a, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), f16::kHighestValue);
    EXPECT_EQ(error(), R"(warning: value 1000000.0 cannot be represented as 'f16')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Convert_F16_TooLow) {
    auto* a = constants.Get(f32(-1000000.0));
    auto result = eval.Convert(create<core::type::F16>(), a, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->ValueAs<f32>(), f16::kLowestValue);
    EXPECT_EQ(error(), R"(warning: value -1000000.0 cannot be represented as 'f16')");
}

TEST_F(ConstEvalRuntimeSemanticsTest, Vec_Overflow_SingleComponent) {
    // Test that overflow for an element-wise vector operation only affects a single component.
    auto* vec4f = create<core::type::Vector>(create<core::type::F32>(), 4u);
    auto* a = eval.VecInitS(vec4f,
                            Vector{
                                constants.Get(f32(1)),
                                constants.Get(f32(4)),
                                constants.Get(f32(-1)),
                                constants.Get(f32(65536)),
                            },
                            {})
                  .Get();
    auto result = eval.sqrt(a->Type(), Vector{a}, {});
    ASSERT_EQ(result, Success);
    EXPECT_EQ(result.Get()->Index(0)->ValueAs<f32>(), 1);
    EXPECT_EQ(result.Get()->Index(1)->ValueAs<f32>(), 2);
    EXPECT_EQ(result.Get()->Index(2)->ValueAs<f32>(), 0);
    EXPECT_EQ(result.Get()->Index(3)->ValueAs<f32>(), 256);
    EXPECT_EQ(error(), R"(warning: sqrt must be called with a value >= 0)");
}

}  // namespace
}  // namespace tint::core::constant::test
