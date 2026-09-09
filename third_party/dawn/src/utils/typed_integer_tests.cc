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

#include <limits>
#include <type_traits>

#include "src/utils/gtest.h"
#include "src/utils/typed_integer.h"
#include "src/utils/underlying_type.h"

namespace dawn {
namespace {

class TypedIntegerTest : public testing::Test {
  protected:
    using Unsigned = TypedInteger<struct UnsignedT, uint32_t>;
    using Signed = TypedInteger<struct SignedT, int32_t>;
};

// Test that typed integers can be created and cast and the internal values are identical
TEST_F(TypedIntegerTest, ConstructionAndCast) {
    Signed svalue(2);
    EXPECT_EQ(static_cast<int32_t>(svalue), 2);

    Unsigned uvalue(7u);
    EXPECT_EQ(static_cast<uint32_t>(uvalue), 7u);

    static_assert(static_cast<int32_t>(Signed(3)) == 3);
    static_assert(static_cast<uint32_t>(Unsigned(28u)) == 28);

    // TypedInteger shouldn't be trivially default constructible, because it is
    // always initialized (initialization is a "non-trivial" operation).
    static_assert(!std::is_trivially_default_constructible_v<Unsigned>);
}

// Test that typed integers can be explicitly cast to other integral types
// (either using a static_cast or a checked_cast depending on the type pair).
TEST_F(TypedIntegerTest, CastToOther) {
    using Unsigned64 = TypedInteger<struct Unsigned64T, uint64_t>;
    using Signed64 = TypedInteger<struct Signed64T, int64_t>;
    using Unsigned32 = TypedInteger<struct Unsigned32T, uint16_t>;
    using Signed32 = TypedInteger<struct Signed32T, int16_t>;
    using Unsigned16 = TypedInteger<struct Unsigned16T, uint16_t>;
    using Signed16 = TypedInteger<struct Signed16T, int16_t>;

    constexpr int32_t maxI32 = std::numeric_limits<int32_t>::max();
    constexpr int16_t maxI16 = std::numeric_limits<int16_t>::max();
    constexpr int8_t maxI8 = std::numeric_limits<int8_t>::max();
    constexpr uint32_t maxU32 = std::numeric_limits<uint32_t>::max();
    constexpr uint16_t maxU16 = std::numeric_limits<uint16_t>::max();
    constexpr uint8_t maxU8 = std::numeric_limits<uint8_t>::max();

    {
        Signed64 svalue64(maxI32);
        EXPECT_EQ(checked_cast<int32_t>(svalue64), maxI32);

        Signed32 svalue32(maxI16);
        EXPECT_EQ(static_cast<int16_t>(svalue32), maxI16);

        Signed16 svalue16(maxI8);
        EXPECT_EQ(checked_cast<int8_t>(svalue16), maxI8);
    }
    {
        Unsigned64 uvalue64(maxU32);
        EXPECT_EQ(checked_cast<uint32_t>(uvalue64), maxU32);

        Unsigned32 uvalue32(maxU16);
        EXPECT_EQ(static_cast<uint16_t>(uvalue32), maxU16);

        Unsigned16 uvalue16(maxU8);
        EXPECT_EQ(checked_cast<uint8_t>(uvalue16), maxU8);
    }
    {
        Signed64 svalue64(maxI8);
        EXPECT_EQ(checked_cast<int32_t>(svalue64), maxI8);
        EXPECT_EQ(checked_cast<int16_t>(svalue64), maxI8);
        EXPECT_EQ(checked_cast<int8_t>(svalue64), maxI8);
    }
    {
        Unsigned64 uvalue64(maxU8);
        EXPECT_EQ(checked_cast<uint32_t>(uvalue64), maxU8);
        EXPECT_EQ(checked_cast<uint16_t>(uvalue64), maxU8);
        EXPECT_EQ(checked_cast<uint8_t>(uvalue64), maxU8);
    }
}

// Test typed integer comparison operators
TEST_F(TypedIntegerTest, Comparison) {
    Unsigned value(8u);

    // Truthy usages of comparison operators
    EXPECT_TRUE(value < Unsigned(9u));
    EXPECT_TRUE(value <= Unsigned(9u));
    EXPECT_TRUE(value <= Unsigned(8u));
    EXPECT_TRUE(value == Unsigned(8u));
    EXPECT_TRUE(value >= Unsigned(8u));
    EXPECT_TRUE(value >= Unsigned(7u));
    EXPECT_TRUE(value > Unsigned(7u));
    EXPECT_TRUE(value != Unsigned(7u));

    // Falsy usages of comparison operators
    EXPECT_FALSE(value >= Unsigned(9u));
    EXPECT_FALSE(value > Unsigned(9u));
    EXPECT_FALSE(value > Unsigned(8u));
    EXPECT_FALSE(value != Unsigned(8u));
    EXPECT_FALSE(value < Unsigned(8u));
    EXPECT_FALSE(value < Unsigned(7u));
    EXPECT_FALSE(value <= Unsigned(7u));
    EXPECT_FALSE(value == Unsigned(7u));
}

TEST_F(TypedIntegerTest, Arithmetic) {
    // Postfix Increment
    {
        Signed value(0);
        EXPECT_EQ(value++, Signed(0));
        EXPECT_EQ(value, Signed(1));
    }

    // Prefix Increment
    {
        Signed value(0);
        EXPECT_EQ(++value, Signed(1));
        EXPECT_EQ(value, Signed(1));
    }

    // Postfix Decrement
    {
        Signed value(0);
        EXPECT_EQ(value--, Signed(0));
        EXPECT_EQ(value, Signed(-1));
    }

    // Prefix Decrement
    {
        Signed value(0);
        EXPECT_EQ(--value, Signed(-1));
        EXPECT_EQ(value, Signed(-1));
    }

    // Signed addition
    {
        Signed a(3);
        Signed b(-4);
        Signed c = a + b;
        EXPECT_EQ(a, Signed(3));
        EXPECT_EQ(b, Signed(-4));
        EXPECT_EQ(c, Signed(-1));
    }

    // Signed subtraction
    {
        Signed a(3);
        Signed b(-4);
        Signed c = a - b;
        EXPECT_EQ(a, Signed(3));
        EXPECT_EQ(b, Signed(-4));
        EXPECT_EQ(c, Signed(7));
    }

    // Unsigned addition
    {
        Unsigned a(9u);
        Unsigned b(3u);
        Unsigned c = a + b;
        EXPECT_EQ(a, Unsigned(9u));
        EXPECT_EQ(b, Unsigned(3u));
        EXPECT_EQ(c, Unsigned(12u));
    }

    // Unsigned subtraction
    {
        Unsigned a(9u);
        Unsigned b(2u);
        Unsigned c = a - b;
        EXPECT_EQ(a, Unsigned(9u));
        EXPECT_EQ(b, Unsigned(2u));
        EXPECT_EQ(c, Unsigned(7u));
    }

    // Negation
    {
        Signed a(5);
        Signed b = -a;
        EXPECT_EQ(a, Signed(5));
        EXPECT_EQ(b, Signed(-5));
    }

    // Signed multiplication
    {
        Signed a(3);
        Signed b(-4);
        Signed c = a * b;
        EXPECT_EQ(a, Signed(3));
        EXPECT_EQ(b, Signed(-4));
        EXPECT_EQ(c, Signed(-12));
    }

    // Unsigned multiplication
    {
        Unsigned a(9u);
        Unsigned b(3u);
        Unsigned c = a * b;
        EXPECT_EQ(a, Unsigned(9u));
        EXPECT_EQ(b, Unsigned(3u));
        EXPECT_EQ(c, Unsigned(27u));
    }

    // Signed division
    {
        Signed a(12);
        Signed b(-4);
        Signed c = a / b;
        EXPECT_EQ(a, Signed(12));
        EXPECT_EQ(b, Signed(-4));
        EXPECT_EQ(c, Signed(-3));
    }

    // Unsigned division
    {
        Unsigned a(12u);
        Unsigned b(3u);
        Unsigned c = a / b;
        EXPECT_EQ(a, Unsigned(12u));
        EXPECT_EQ(b, Unsigned(3u));
        EXPECT_EQ(c, Unsigned(4u));
    }

    // Signed modulo
    {
        Signed a(12);
        Signed b(-5);
        Signed c = a % b;
        EXPECT_EQ(a, Signed(12));
        EXPECT_EQ(b, Signed(-5));
        EXPECT_EQ(c, Signed(2));
    }

    // Unsigned modulo
    {
        Unsigned a(12u);
        Unsigned b(5u);
        Unsigned c = a % b;
        EXPECT_EQ(a, Unsigned(12u));
        EXPECT_EQ(b, Unsigned(5u));
        EXPECT_EQ(c, Unsigned(2u));
    }
}

TEST_F(TypedIntegerTest, ArithmeticAssignment) {
    // Signed addition assignment
    {
        Signed a(3);
        Signed b(-4);
        a += b;
        EXPECT_EQ(a, Signed(-1));
        EXPECT_EQ(b, Signed(-4));
    }

    // Signed subtraction assignment
    {
        Signed a(3);
        Signed b(-4);
        a -= b;
        EXPECT_EQ(a, Signed(7));
        EXPECT_EQ(b, Signed(-4));
    }

    // Unsigned addition assignment
    {
        Unsigned a(9u);
        Unsigned b(3u);
        a += b;
        EXPECT_EQ(a, Unsigned(12u));
        EXPECT_EQ(b, Unsigned(3u));
    }

    // Unsigned subtraction assignment
    {
        Unsigned a(9u);
        Unsigned b(2u);
        a -= b;
        EXPECT_EQ(a, Unsigned(7u));
        EXPECT_EQ(b, Unsigned(2u));
    }

    // Signed multiplication assignment
    {
        Signed a(3);
        Signed b(-4);
        a *= b;
        EXPECT_EQ(a, Signed(-12));
        EXPECT_EQ(b, Signed(-4));
    }

    // Unsigned multiplication assignment
    {
        Unsigned a(9u);
        Unsigned b(3u);
        a *= b;
        EXPECT_EQ(a, Unsigned(27u));
        EXPECT_EQ(b, Unsigned(3u));
    }

    // Signed division assignment
    {
        Signed a(12);
        Signed b(-4);
        a /= b;
        EXPECT_EQ(a, Signed(-3));
        EXPECT_EQ(b, Signed(-4));
    }

    // Unsigned division assignment
    {
        Unsigned a(12u);
        Unsigned b(3u);
        a /= b;
        EXPECT_EQ(a, Unsigned(4u));
        EXPECT_EQ(b, Unsigned(3u));
    }

    // Signed modulo assignment
    {
        Signed a(12);
        Signed b(-5);
        a %= b;
        EXPECT_EQ(a, Signed(2));
        EXPECT_EQ(b, Signed(-5));
    }

    // Unsigned modulo assignment
    {
        Unsigned a(12u);
        Unsigned b(5u);
        a %= b;
        EXPECT_EQ(a, Unsigned(2u));
        EXPECT_EQ(b, Unsigned(5u));
    }
}

TEST_F(TypedIntegerTest, PlusOne) {
    EXPECT_EQ(Unsigned(11u).PlusOne(), Unsigned(12u));
    EXPECT_EQ(Signed(11).PlusOne(), Signed(12));
    EXPECT_EQ(Signed(-1).PlusOne(), Signed(0));
}

TEST_F(TypedIntegerTest, MinusOne) {
    EXPECT_EQ(Unsigned(11u).MinusOne(), Unsigned(10u));
    EXPECT_EQ(Signed(11).MinusOne(), Signed(10));
    EXPECT_EQ(Signed(1).MinusOne(), Signed(0));
}

TEST_F(TypedIntegerTest, NumericLimits) {
    EXPECT_EQ(std::numeric_limits<Unsigned>::max(), Unsigned(std::numeric_limits<uint32_t>::max()));
    EXPECT_EQ(std::numeric_limits<Unsigned>::min(), Unsigned(std::numeric_limits<uint32_t>::min()));
    EXPECT_EQ(std::numeric_limits<Signed>::max(), Signed(std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(std::numeric_limits<Signed>::min(), Signed(std::numeric_limits<int32_t>::min()));
}

TEST_F(TypedIntegerTest, NotIntegral) {
    static_assert(!std::is_integral_v<Unsigned>);
    static_assert(!std::is_integral_v<Signed>);
}

TEST_F(TypedIntegerTest, UnderlyingType) {
    static_assert(std::is_same_v<UnderlyingType<Unsigned>, uint32_t>);
    static_assert(std::is_same_v<UnderlyingType<Signed>, int32_t>);
}

TEST_F(TypedIntegerTest, HasUnsignedUnderlyingType) {
    static_assert(!HasUnsignedUnderlyingType<Signed>);
    static_assert(HasUnsignedUnderlyingType<Unsigned>);
}

// Name "*DeathTest" per https://google.github.io/googletest/advanced.html#death-test-naming
using TypedIntegerDeathTest = TypedIntegerTest;

// Tests for bounds assertions on arithmetic overflow and underflow.
// Note (d)checked_cast tests are in NumericDeathTest.

TEST_F(TypedIntegerDeathTest, IncrementUnsignedOverflow) {
    Unsigned value(std::numeric_limits<uint32_t>::max() - 1);

    value++;                    // Doesn't overflow.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value++, "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, IncrementSignedOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() - 1);

    value++;                    // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value++, "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, DecrementUnsignedUnderflow) {
    Unsigned value(std::numeric_limits<uint32_t>::min() + 1);

    value--;                    // Doesn't underflow.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value--, "");  // Underflows.
}

TEST_F(TypedIntegerDeathTest, DecrementSignedUnderflow) {
    Signed value(std::numeric_limits<int32_t>::min() + 1);

    value--;                    // Doesn't underflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value--, "");  // Underflows.
}

TEST_F(TypedIntegerDeathTest, UnsignedAdditionOverflow) {
    Unsigned value(std::numeric_limits<uint32_t>::max() - 1);

    value + Unsigned(1u);                     // Doesn't overflow.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value + Unsigned(2u), "");   // Overflows.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value += Unsigned(2u), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, SignedAdditionOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() - 1);

    value + Signed(1);                     // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value + Signed(2), "");   // Overflows.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value += Signed(2), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, SignedAdditionUnderflow) {
    Signed value(std::numeric_limits<int32_t>::min() + 1);

    value + Signed(-1);                     // Doesn't underflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value + Signed(-2), "");   // Underflows.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value += Signed(-2), "");  // Underflows.
}

TEST_F(TypedIntegerDeathTest, UnsignedSubtractionUnderflow) {
    Unsigned value(1u);

    value - Unsigned(1u);                     // Doesn't underflow.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value - Unsigned(2u), "");   // Underflows.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value -= Unsigned(2u), "");  // Underflows.
}

TEST_F(TypedIntegerDeathTest, SignedSubtractionOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() - 1);

    value - Signed(-1);                     // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value - Signed(-2), "");   // Overflows.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value -= Signed(-2), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, SignedSubtractionUnderflow) {
    Signed value(std::numeric_limits<int32_t>::min() + 1);

    value - Signed(1);                     // Doesn't underflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value - Signed(2), "");   // Underflows.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value -= Signed(2), "");  // Underflows.
}

TEST_F(TypedIntegerDeathTest, UnsignedMultiplicationOverflow) {
    Unsigned value(std::numeric_limits<uint32_t>::max() / 2);

    value* Unsigned(2u);                      // Doesn't overflow.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value * Unsigned(3u), "");   // Overflows.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value *= Unsigned(3u), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, SignedMultiplicationOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() / 2);

    value* Signed(2);                      // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value * Signed(3), "");   // Overflows.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value *= Signed(3), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, SignedMultiplicationUnderflow) {
    Signed value(std::numeric_limits<int32_t>::min() / 2);

    value* Signed(2);                      // Doesn't underflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value * Signed(3), "");   // Underflows.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value *= Signed(3), "");  // Underflows.
}

TEST_F(TypedIntegerDeathTest, UnsignedDivisionByZero) {
    Unsigned value(1u);

    value / Unsigned(1u);                     // Doesn't underflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value / Unsigned(0u), "");   // DBZ.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value /= Unsigned(0u), "");  // DBZ.
}

TEST_F(TypedIntegerDeathTest, SignedDivisionByZero) {
    Signed value(1);

    value / Signed(-1);                    // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value / Signed(0), "");   // DBZ.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value /= Signed(0), "");  // DBZ.
}

TEST_F(TypedIntegerDeathTest, SignedDivisionOverflow) {
    // Overflow can also occur during two's complement signed integer division when the dividend is
    // equal to the minimum (most negative) value for the signed integer type and the divisor is
    // equal to −1.
    Signed value(std::numeric_limits<int32_t>::min());

    value / Signed(1);                      // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value / Signed(-1), "");   // Overflows.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value /= Signed(-1), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, UnsignedModuloByZero) {
    Unsigned value(1u);

    value % Unsigned(1u);                     // Doesn't underflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value % Unsigned(0u), "");   // DBZ.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value %= Unsigned(0u), "");  // DBZ.
}

TEST_F(TypedIntegerDeathTest, SignedModuloByZero) {
    Signed value(1);

    value % Signed(-1);                    // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value % Signed(0), "");   // DBZ.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value %= Signed(0), "");  // DBZ.
}

TEST_F(TypedIntegerDeathTest, SignedModuloOverflow) {
    // Overflow can also occur during two's complement signed integer modulo when the dividend is
    // equal to the minimum (most negative) value for the signed integer type and the divisor is
    // equal to −1.
    Signed value(std::numeric_limits<int32_t>::min());

    value % Signed(1);                      // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value % Signed(-1), "");   // Overflows.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value %= Signed(-1), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, NegationOverflow) {
    Signed maxValue(std::numeric_limits<int32_t>::max());
    -maxValue;  // Doesn't underflow.

    Signed minValue(std::numeric_limits<int32_t>::min());
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(-minValue, "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, UnsignedPlusOneOverflow) {
    Unsigned value(std::numeric_limits<uint32_t>::max() - 1);

    value.PlusOne();                              // Doesn't overflow.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value.PlusOne().PlusOne(), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, SignedPlusOneOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() - 1);

    value.PlusOne();                              // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value.PlusOne().PlusOne(),
                                                          "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, UnsignedMinusOneOverflow) {
    Unsigned value(std::numeric_limits<uint32_t>::lowest() + 1);

    value.MinusOne();                               // Doesn't overflow.
    DAWN_EXPECT_DEBUG_DEATH_IF_SUPPORTED(value.MinusOne().MinusOne(), "");  // Overflows.
}

TEST_F(TypedIntegerDeathTest, SignedMinusOneOverflow) {
    Signed value(std::numeric_limits<int32_t>::lowest() + 1);

    value.MinusOne();                               // Doesn't overflow.
    DAWN_EXPECT_ASSERT_DEATH_IF_SUPPORTED_ELSE_SKIP_UBSAN(value.MinusOne().MinusOne(),
                                                          "");  // Overflows.
}

}  // anonymous namespace
}  // namespace dawn
