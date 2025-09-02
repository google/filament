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

#include "dawn/common/TypedInteger.h"
#include "dawn/common/UnderlyingType.h"
#include "gtest/gtest.h"

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

    Unsigned uvalue(7);
    EXPECT_EQ(static_cast<uint32_t>(uvalue), 7u);

    static_assert(static_cast<int32_t>(Signed(3)) == 3);
    static_assert(static_cast<uint32_t>(Unsigned(28)) == 28);
}

// Test that typed integers can be explicitly cast to other integral types
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
        EXPECT_EQ(static_cast<int32_t>(svalue64), maxI32);

        Signed32 svalue32(maxI16);
        EXPECT_EQ(static_cast<int16_t>(svalue32), maxI16);

        Signed16 svalue16(maxI8);
        EXPECT_EQ(static_cast<int8_t>(svalue16), maxI8);
    }
    {
        Unsigned64 uvalue64(maxU32);
        EXPECT_EQ(static_cast<uint32_t>(uvalue64), maxU32);

        Unsigned32 uvalue32(maxU16);
        EXPECT_EQ(static_cast<uint16_t>(uvalue32), maxU16);

        Unsigned16 uvalue16(maxU8);
        EXPECT_EQ(static_cast<uint8_t>(uvalue16), maxU8);
    }
    {
        Signed64 svalue64(maxI8);
        EXPECT_EQ(static_cast<int32_t>(svalue64), maxI8);
        EXPECT_EQ(static_cast<int16_t>(svalue64), maxI8);
        EXPECT_EQ(static_cast<int8_t>(svalue64), maxI8);
    }
    {
        Unsigned64 uvalue64(maxU8);
        EXPECT_EQ(static_cast<uint32_t>(uvalue64), maxU8);
        EXPECT_EQ(static_cast<uint16_t>(uvalue64), maxU8);
        EXPECT_EQ(static_cast<uint8_t>(uvalue64), maxU8);
    }
}

// Test typed integer comparison operators
TEST_F(TypedIntegerTest, Comparison) {
    Unsigned value(8);

    // Truthy usages of comparison operators
    EXPECT_TRUE(value < Unsigned(9));
    EXPECT_TRUE(value <= Unsigned(9));
    EXPECT_TRUE(value <= Unsigned(8));
    EXPECT_TRUE(value == Unsigned(8));
    EXPECT_TRUE(value >= Unsigned(8));
    EXPECT_TRUE(value >= Unsigned(7));
    EXPECT_TRUE(value > Unsigned(7));
    EXPECT_TRUE(value != Unsigned(7));

    // Falsy usages of comparison operators
    EXPECT_FALSE(value >= Unsigned(9));
    EXPECT_FALSE(value > Unsigned(9));
    EXPECT_FALSE(value > Unsigned(8));
    EXPECT_FALSE(value != Unsigned(8));
    EXPECT_FALSE(value < Unsigned(8));
    EXPECT_FALSE(value < Unsigned(7));
    EXPECT_FALSE(value <= Unsigned(7));
    EXPECT_FALSE(value == Unsigned(7));
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
        Unsigned a(9);
        Unsigned b(3);
        Unsigned c = a + b;
        EXPECT_EQ(a, Unsigned(9));
        EXPECT_EQ(b, Unsigned(3));
        EXPECT_EQ(c, Unsigned(12));
    }

    // Unsigned subtraction
    {
        Unsigned a(9);
        Unsigned b(2);
        Unsigned c = a - b;
        EXPECT_EQ(a, Unsigned(9));
        EXPECT_EQ(b, Unsigned(2));
        EXPECT_EQ(c, Unsigned(7));
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
        Unsigned a(9);
        Unsigned b(3);
        Unsigned c = a * b;
        EXPECT_EQ(a, Unsigned(9));
        EXPECT_EQ(b, Unsigned(3));
        EXPECT_EQ(c, Unsigned(27));
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
        Unsigned a(12);
        Unsigned b(3);
        Unsigned c = a / b;
        EXPECT_EQ(a, Unsigned(12));
        EXPECT_EQ(b, Unsigned(3));
        EXPECT_EQ(c, Unsigned(4));
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
        Unsigned a(12);
        Unsigned b(5);
        Unsigned c = a % b;
        EXPECT_EQ(a, Unsigned(12));
        EXPECT_EQ(b, Unsigned(5));
        EXPECT_EQ(c, Unsigned(2));
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
        Unsigned a(9);
        Unsigned b(3);
        a += b;
        EXPECT_EQ(a, Unsigned(12));
        EXPECT_EQ(b, Unsigned(3));
    }

    // Unsigned subtraction assignment
    {
        Unsigned a(9);
        Unsigned b(2);
        a -= b;
        EXPECT_EQ(a, Unsigned(7));
        EXPECT_EQ(b, Unsigned(2));
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
        Unsigned a(9);
        Unsigned b(3);
        a *= b;
        EXPECT_EQ(a, Unsigned(27));
        EXPECT_EQ(b, Unsigned(3));
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
        Unsigned a(12);
        Unsigned b(3);
        a /= b;
        EXPECT_EQ(a, Unsigned(4));
        EXPECT_EQ(b, Unsigned(3));
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
        Unsigned a(12);
        Unsigned b(5);
        a %= b;
        EXPECT_EQ(a, Unsigned(2));
        EXPECT_EQ(b, Unsigned(5));
    }
}

TEST_F(TypedIntegerTest, NumericLimits) {
    EXPECT_EQ(std::numeric_limits<Unsigned>::max(), Unsigned(std::numeric_limits<uint32_t>::max()));
    EXPECT_EQ(std::numeric_limits<Unsigned>::min(), Unsigned(std::numeric_limits<uint32_t>::min()));
    EXPECT_EQ(std::numeric_limits<Signed>::max(), Signed(std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(std::numeric_limits<Signed>::min(), Signed(std::numeric_limits<int32_t>::min()));
}

TEST_F(TypedIntegerTest, UnderlyingType) {
    static_assert(std::is_same<UnderlyingType<Unsigned>, uint32_t>::value);
    static_assert(std::is_same<UnderlyingType<Signed>, int32_t>::value);
}

TEST_F(TypedIntegerTest, PlusOne) {
    Signed seven(7);
    EXPECT_EQ(Signed(8), ityp::PlusOne(seven));
    EXPECT_EQ(Signed(9), ityp::PlusOne(ityp::PlusOne(seven)));
}

// Tests for bounds assertions on arithmetic overflow and underflow.
#if defined(DAWN_ENABLE_ASSERTS)

TEST_F(TypedIntegerTest, CastToOtherTruncation) {
    using Unsigned64 = TypedInteger<struct Unsigned64T, uint64_t>;
    using Signed64 = TypedInteger<struct Signed64T, int64_t>;

    constexpr int32_t maxI32 = std::numeric_limits<int32_t>::max();
    constexpr uint32_t maxU32 = std::numeric_limits<uint32_t>::max();
    constexpr int32_t minI32 = std::numeric_limits<int32_t>::min();

    Signed64 too_large_for_i32(static_cast<int64_t>(maxI32) + 1);
    Signed64 too_small_for_i32(static_cast<int64_t>(minI32) - 1);
    Unsigned64 too_large_for_u32(static_cast<uint64_t>(maxU32) + 1);

    EXPECT_DEATH({ [[maybe_unused]] auto result = static_cast<int32_t>(too_large_for_i32); }, "");
    EXPECT_DEATH({ [[maybe_unused]] auto result = static_cast<int32_t>(too_small_for_i32); }, "");
    EXPECT_DEATH({ [[maybe_unused]] auto result = static_cast<uint32_t>(too_large_for_u32); }, "");
}

TEST_F(TypedIntegerTest, IncrementUnsignedOverflow) {
    Unsigned value(std::numeric_limits<uint32_t>::max() - 1);

    value++;                    // Doesn't overflow.
    EXPECT_DEATH(value++, "");  // Overflows.
}

TEST_F(TypedIntegerTest, IncrementSignedOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() - 1);

    value++;                    // Doesn't overflow.
    EXPECT_DEATH(value++, "");  // Overflows.
}

TEST_F(TypedIntegerTest, DecrementUnsignedUnderflow) {
    Unsigned value(std::numeric_limits<uint32_t>::min() + 1);

    value--;                    // Doesn't underflow.
    EXPECT_DEATH(value--, "");  // Underflows.
}

TEST_F(TypedIntegerTest, DecrementSignedUnderflow) {
    Signed value(std::numeric_limits<int32_t>::min() + 1);

    value--;                    // Doesn't underflow.
    EXPECT_DEATH(value--, "");  // Underflows.
}

TEST_F(TypedIntegerTest, UnsignedAdditionOverflow) {
    Unsigned value(std::numeric_limits<uint32_t>::max() - 1);

    value + Unsigned(1);                    // Doesn't overflow.
    EXPECT_DEATH(value + Unsigned(2), "");  // Overflows.
    EXPECT_DEATH(value += Unsigned(2), "");  // Overflows.
}

TEST_F(TypedIntegerTest, SignedAdditionOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() - 1);

    value + Signed(1);                    // Doesn't overflow.
    EXPECT_DEATH(value + Signed(2), "");  // Overflows.
    EXPECT_DEATH(value += Signed(2), "");  // Overflows.
}

TEST_F(TypedIntegerTest, SignedAdditionUnderflow) {
    Signed value(std::numeric_limits<int32_t>::min() + 1);

    value + Signed(-1);                    // Doesn't underflow.
    EXPECT_DEATH(value + Signed(-2), "");  // Underflows.
    EXPECT_DEATH(value += Signed(-2), "");  // Underflows.
}

TEST_F(TypedIntegerTest, UnsignedSubtractionUnderflow) {
    Unsigned value(1);

    value - Unsigned(1);                     // Doesn't underflow.
    EXPECT_DEATH(value - Unsigned(2), "");   // Underflows.
    EXPECT_DEATH(value -= Unsigned(2), "");  // Underflows.
}

TEST_F(TypedIntegerTest, SignedSubtractionOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() - 1);

    value - Signed(-1);                    // Doesn't overflow.
    EXPECT_DEATH(value - Signed(-2), "");  // Overflows.
    EXPECT_DEATH(value -= Signed(-2), "");  // Overflows.
}

TEST_F(TypedIntegerTest, SignedSubtractionUnderflow) {
    Signed value(std::numeric_limits<int32_t>::min() + 1);

    value - Signed(1);                    // Doesn't underflow.
    EXPECT_DEATH(value - Signed(2), "");  // Underflows.
    EXPECT_DEATH(value -= Signed(2), "");  // Underflows.
}

TEST_F(TypedIntegerTest, UnsignedMultiplicationOverflow) {
    Unsigned value(std::numeric_limits<uint32_t>::max() / 2);

    value* Unsigned(2);                      // Doesn't overflow.
    EXPECT_DEATH(value * Unsigned(3), "");   // Overflows.
    EXPECT_DEATH(value *= Unsigned(3), "");  // Overflows.
}

TEST_F(TypedIntegerTest, SignedMultiplicationOverflow) {
    Signed value(std::numeric_limits<int32_t>::max() / 2);

    value* Signed(2);                      // Doesn't overflow.
    EXPECT_DEATH(value * Signed(3), "");   // Overflows.
    EXPECT_DEATH(value *= Signed(3), "");  // Overflows.
}

TEST_F(TypedIntegerTest, SignedMultiplicationUnderflow) {
    Signed value(std::numeric_limits<int32_t>::min() / 2);

    value* Signed(2);                      // Doesn't underflow.
    EXPECT_DEATH(value * Signed(3), "");   // Underflows.
    EXPECT_DEATH(value *= Signed(3), "");  // Underflows.
}

TEST_F(TypedIntegerTest, UnsignedDivisionByZero) {
    Unsigned value(1);

    value / Unsigned(1);                     // Doesn't underflow.
    EXPECT_DEATH(value / Unsigned(0), "");   // DBZ.
    EXPECT_DEATH(value /= Unsigned(0), "");  // DBZ.
}

TEST_F(TypedIntegerTest, SignedDivisionByZero) {
    Signed value(1);

    value / Signed(-1);                    // Doesn't overflow.
    EXPECT_DEATH(value / Signed(0), "");   // DBZ.
    EXPECT_DEATH(value /= Signed(0), "");  // DBZ.
}

TEST_F(TypedIntegerTest, SignedDivisionOverflow) {
    // Overflow can also occur during two's complement signed integer division when the dividend is
    // equal to the minimum (most negative) value for the signed integer type and the divisor is
    // equal to −1.
    Signed value(std::numeric_limits<int32_t>::min());

    value / Signed(1);                      // Doesn't overflow.
    EXPECT_DEATH(value / Signed(-1), "");   // Overflows.
    EXPECT_DEATH(value /= Signed(-1), "");  // Overflows.
}

TEST_F(TypedIntegerTest, UnsignedModuloByZero) {
    Unsigned value(1);

    value % Unsigned(1);                     // Doesn't underflow.
    EXPECT_DEATH(value % Unsigned(0), "");   // DBZ.
    EXPECT_DEATH(value %= Unsigned(0), "");  // DBZ.
}

TEST_F(TypedIntegerTest, SignedModuloByZero) {
    Signed value(1);

    value % Signed(-1);                    // Doesn't overflow.
    EXPECT_DEATH(value % Signed(0), "");   // DBZ.
    EXPECT_DEATH(value %= Signed(0), "");  // DBZ.
}

TEST_F(TypedIntegerTest, SignedModuloOverflow) {
    // Overflow can also occur during two's complement signed integer modulo when the dividend is
    // equal to the minimum (most negative) value for the signed integer type and the divisor is
    // equal to −1.
    Signed value(std::numeric_limits<int32_t>::min());

    value % Signed(1);                      // Doesn't overflow.
    EXPECT_DEATH(value % Signed(-1), "");   // Overflows.
    EXPECT_DEATH(value %= Signed(-1), "");  // Overflows.
}

TEST_F(TypedIntegerTest, NegationOverflow) {
    Signed maxValue(std::numeric_limits<int32_t>::max());
    -maxValue;  // Doesn't underflow.

    Signed minValue(std::numeric_limits<int32_t>::min());
    EXPECT_DEATH(-minValue, "");  // Overflows.
}

#endif  // defined(DAWN_ENABLE_ASSERTS)

}  // anonymous namespace
}  // namespace dawn
