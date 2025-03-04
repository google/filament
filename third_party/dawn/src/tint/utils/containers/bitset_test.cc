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

#include "src/tint/utils/containers/bitset.h"

#include "gtest/gtest.h"

namespace tint {
namespace {

TEST(Bitset, Length) {
    Bitset<8> bits;
    EXPECT_EQ(bits.Length(), 0u);
    bits.Resize(100u);
    EXPECT_EQ(bits.Length(), 100u);
}

TEST(Bitset, AllBitsZero) {
    Bitset<8> bits;
    EXPECT_TRUE(bits.AllBitsZero());

    bits.Resize(4u);
    EXPECT_TRUE(bits.AllBitsZero());

    bits.Resize(100u);
    EXPECT_TRUE(bits.AllBitsZero());

    bits[63] = true;
    EXPECT_FALSE(bits.AllBitsZero());

    bits.Resize(60);
    EXPECT_TRUE(bits.AllBitsZero());

    bits.Resize(64);
    EXPECT_TRUE(bits.AllBitsZero());

    bits[4] = true;
    EXPECT_FALSE(bits.AllBitsZero());

    bits.Resize(8);
    EXPECT_FALSE(bits.AllBitsZero());
}

TEST(Bitset, InitCleared_NoSpill) {
    Bitset<256> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_FALSE(bits[i]);
    }
}

TEST(Bitset, InitCleared_Spill) {
    Bitset<64> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_FALSE(bits[i]);
    }
}

TEST(Bitset, ReadWrite_NoSpill) {
    Bitset<256> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = (i & 0x2) == 0;
    }
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], (i & 0x2) == 0);
    }
}

TEST(Bitset, ReadWrite_Spill) {
    Bitset<64> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = (i & 0x2) == 0;
    }
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], (i & 0x2) == 0);
    }
}

TEST(Bitset, ShinkGrowAlignedClears_NoSpill) {
    Bitset<256> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = true;
    }
    bits.Resize(64);
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], i < 64u);
    }
}

TEST(Bitset, ShinkGrowAlignedClears_Spill) {
    Bitset<64> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = true;
    }
    bits.Resize(64);
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], i < 64u);
    }
}
TEST(Bitset, ShinkGrowMisalignedClears_NoSpill) {
    Bitset<256> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = true;
    }
    bits.Resize(42);
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], i < 42u);
    }
}

TEST(Bitset, ShinkGrowMisalignedClears_Spill) {
    Bitset<64> bits;
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        bits[i] = true;
    }
    bits.Resize(42);
    bits.Resize(256);
    for (size_t i = 0; i < 256; i++) {
        EXPECT_EQ(bits[i], i < 42u);
    }
}

}  // namespace
}  // namespace tint
