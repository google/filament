// Copyright 2021 The Dawn & Tint Authors
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

#include "src/tint/utils/math/math.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(MathTests, RoundUp) {
    EXPECT_EQ(RoundUp(1, 0), 0);
    EXPECT_EQ(RoundUp(1, 1), 1);
    EXPECT_EQ(RoundUp(1, 2), 2);

    EXPECT_EQ(RoundUp(1, 1), 1);
    EXPECT_EQ(RoundUp(2, 1), 2);
    EXPECT_EQ(RoundUp(3, 1), 3);
    EXPECT_EQ(RoundUp(4, 1), 4);

    EXPECT_EQ(RoundUp(1, 2), 2);
    EXPECT_EQ(RoundUp(2, 2), 2);
    EXPECT_EQ(RoundUp(3, 2), 3);
    EXPECT_EQ(RoundUp(4, 2), 4);

    EXPECT_EQ(RoundUp(1, 3), 3);
    EXPECT_EQ(RoundUp(2, 3), 4);
    EXPECT_EQ(RoundUp(3, 3), 3);
    EXPECT_EQ(RoundUp(4, 3), 4);

    EXPECT_EQ(RoundUp(1, 4), 4);
    EXPECT_EQ(RoundUp(2, 4), 4);
    EXPECT_EQ(RoundUp(3, 4), 6);
    EXPECT_EQ(RoundUp(4, 4), 4);
}

TEST(MathTests, IsPowerOfTwo) {
    EXPECT_EQ(IsPowerOfTwo(1), true);
    EXPECT_EQ(IsPowerOfTwo(2), true);
    EXPECT_EQ(IsPowerOfTwo(3), false);
    EXPECT_EQ(IsPowerOfTwo(4), true);
    EXPECT_EQ(IsPowerOfTwo(5), false);
    EXPECT_EQ(IsPowerOfTwo(6), false);
    EXPECT_EQ(IsPowerOfTwo(7), false);
    EXPECT_EQ(IsPowerOfTwo(8), true);
    EXPECT_EQ(IsPowerOfTwo(9), false);
}

TEST(MathTests, Log2) {
    EXPECT_EQ(Log2(1), 0u);
    EXPECT_EQ(Log2(2), 1u);
    EXPECT_EQ(Log2(3), 1u);
    EXPECT_EQ(Log2(4), 2u);
    EXPECT_EQ(Log2(5), 2u);
    EXPECT_EQ(Log2(6), 2u);
    EXPECT_EQ(Log2(7), 2u);
    EXPECT_EQ(Log2(8), 3u);
    EXPECT_EQ(Log2(9), 3u);
    EXPECT_EQ(Log2(0x7fffffffu), 30u);
    EXPECT_EQ(Log2(0x80000000u), 31u);
    EXPECT_EQ(Log2(0x80000001u), 31u);
    EXPECT_EQ(Log2(0x7fffffffffffffffu), 62u);
    EXPECT_EQ(Log2(0x8000000000000000u), 63u);

    static_assert(Log2(1) == 0u);
    static_assert(Log2(2) == 1u);
    static_assert(Log2(3) == 1u);
    static_assert(Log2(4) == 2u);
    static_assert(Log2(5) == 2u);
    static_assert(Log2(6) == 2u);
    static_assert(Log2(7) == 2u);
    static_assert(Log2(8) == 3u);
    static_assert(Log2(9) == 3u);
    static_assert(Log2(0x7fffffffu) == 30u);
    static_assert(Log2(0x80000000u) == 31u);
    static_assert(Log2(0x80000001u) == 31u);
    static_assert(Log2(0x7fffffffffffffffu) == 62u);
    static_assert(Log2(0x8000000000000000u) == 63u);
}

TEST(MathTests, NextPowerOfTwo) {
    EXPECT_EQ(NextPowerOfTwo(0), 1u);
    EXPECT_EQ(NextPowerOfTwo(1), 1u);
    EXPECT_EQ(NextPowerOfTwo(2), 2u);
    EXPECT_EQ(NextPowerOfTwo(3), 4u);
    EXPECT_EQ(NextPowerOfTwo(4), 4u);
    EXPECT_EQ(NextPowerOfTwo(5), 8u);
    EXPECT_EQ(NextPowerOfTwo(6), 8u);
    EXPECT_EQ(NextPowerOfTwo(7), 8u);
    EXPECT_EQ(NextPowerOfTwo(8), 8u);
    EXPECT_EQ(NextPowerOfTwo(9), 16u);
    EXPECT_EQ(NextPowerOfTwo(0x7fffffffu), 0x80000000u);
    EXPECT_EQ(NextPowerOfTwo(0x80000000u), 0x80000000u);
    EXPECT_EQ(NextPowerOfTwo(0x80000001u), 0x100000000u);
    EXPECT_EQ(NextPowerOfTwo(0x7fffffffffffffffu), 0x8000000000000000u);

    static_assert(NextPowerOfTwo(0) == 1u);
    static_assert(NextPowerOfTwo(1) == 1u);
    static_assert(NextPowerOfTwo(2) == 2u);
    static_assert(NextPowerOfTwo(3) == 4u);
    static_assert(NextPowerOfTwo(4) == 4u);
    static_assert(NextPowerOfTwo(5) == 8u);
    static_assert(NextPowerOfTwo(6) == 8u);
    static_assert(NextPowerOfTwo(7) == 8u);
    static_assert(NextPowerOfTwo(8) == 8u);
    static_assert(NextPowerOfTwo(9) == 16u);
    static_assert(NextPowerOfTwo(0x7fffffffu) == 0x80000000u);
    static_assert(NextPowerOfTwo(0x80000000u) == 0x80000000u);
    static_assert(NextPowerOfTwo(0x80000001u) == 0x100000000u);
    static_assert(NextPowerOfTwo(0x7fffffffffffffffu) == 0x8000000000000000u);
}

TEST(MathTests, MaxAlignOf) {
    EXPECT_EQ(MaxAlignOf(0u), 1u);
    EXPECT_EQ(MaxAlignOf(1u), 1u);
    EXPECT_EQ(MaxAlignOf(2u), 2u);
    EXPECT_EQ(MaxAlignOf(3u), 1u);
    EXPECT_EQ(MaxAlignOf(4u), 4u);
    EXPECT_EQ(MaxAlignOf(5u), 1u);
    EXPECT_EQ(MaxAlignOf(6u), 2u);
    EXPECT_EQ(MaxAlignOf(7u), 1u);
    EXPECT_EQ(MaxAlignOf(8u), 8u);
    EXPECT_EQ(MaxAlignOf(9u), 1u);
    EXPECT_EQ(MaxAlignOf(10u), 2u);
    EXPECT_EQ(MaxAlignOf(11u), 1u);
    EXPECT_EQ(MaxAlignOf(12u), 4u);
    EXPECT_EQ(MaxAlignOf(13u), 1u);
    EXPECT_EQ(MaxAlignOf(14u), 2u);
    EXPECT_EQ(MaxAlignOf(15u), 1u);
    EXPECT_EQ(MaxAlignOf(16u), 16u);
}

}  // namespace
}  // namespace tint
