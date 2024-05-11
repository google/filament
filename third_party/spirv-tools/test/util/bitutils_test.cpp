// Copyright (c) 2019 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "source/util/bitutils.h"

#include "gmock/gmock.h"

namespace spvtools {
namespace utils {
namespace {

using BitUtilsTest = ::testing::Test;

TEST(BitUtilsTest, MutateBitsWholeWord) {
  const uint32_t zero_u32 = 0;
  const uint32_t max_u32 = ~0;

  EXPECT_EQ(MutateBits(zero_u32, 0, 0, false), zero_u32);
  EXPECT_EQ(MutateBits(max_u32, 0, 0, false), max_u32);
  EXPECT_EQ(MutateBits(zero_u32, 0, 32, false), zero_u32);
  EXPECT_EQ(MutateBits(zero_u32, 0, 32, true), max_u32);
  EXPECT_EQ(MutateBits(max_u32, 0, 32, true), max_u32);
  EXPECT_EQ(MutateBits(max_u32, 0, 32, false), zero_u32);
}

TEST(BitUtilsTest, MutateBitsLow) {
  const uint32_t zero_u32 = 0;
  const uint32_t one_u32 = 1;
  const uint32_t max_u32 = ~0;

  EXPECT_EQ(MutateBits(zero_u32, 0, 1, false), zero_u32);
  EXPECT_EQ(MutateBits(zero_u32, 0, 1, true), one_u32);
  EXPECT_EQ(MutateBits(max_u32, 0, 1, true), max_u32);
  EXPECT_EQ(MutateBits(one_u32, 0, 32, false), zero_u32);
  EXPECT_EQ(MutateBits(one_u32, 0, 1, true), one_u32);
  EXPECT_EQ(MutateBits(one_u32, 0, 1, false), zero_u32);
  EXPECT_EQ(MutateBits(zero_u32, 0, 3, true), uint32_t(7));
  EXPECT_EQ(MutateBits(uint32_t(7), 0, 2, false), uint32_t(4));
}

TEST(BitUtilsTest, MutateBitsHigh) {
  const uint8_t zero_u8 = 0;
  const uint8_t one_u8 = 1;
  const uint8_t max_u8 = 255;

  EXPECT_EQ(MutateBits(zero_u8, 7, 0, true), zero_u8);
  EXPECT_EQ(MutateBits(zero_u8, 7, 1, true), uint8_t(128));
  EXPECT_EQ(MutateBits(one_u8, 7, 1, true), uint8_t(129));
  EXPECT_EQ(MutateBits(max_u8, 7, 1, true), max_u8);
  EXPECT_EQ(MutateBits(max_u8, 7, 1, false), uint8_t(127));
  EXPECT_EQ(MutateBits(max_u8, 6, 2, true), max_u8);
  EXPECT_EQ(MutateBits(max_u8, 6, 2, false), uint8_t(63));
}

TEST(BitUtilsTest, MutateBitsUint8Mid) {
  const uint8_t zero_u8 = 0;
  const uint8_t max_u8 = 255;

  EXPECT_EQ(MutateBits(zero_u8, 1, 2, true), uint8_t(6));
  EXPECT_EQ(MutateBits(max_u8, 1, 2, true), max_u8);
  EXPECT_EQ(MutateBits(max_u8, 1, 2, false), uint8_t(0xF9));
  EXPECT_EQ(MutateBits(zero_u8, 2, 3, true), uint8_t(0x1C));
}

TEST(BitUtilsTest, MutateBitsUint64Mid) {
  const uint64_t zero_u64 = 0;
  const uint64_t max_u64 = ~zero_u64;

  EXPECT_EQ(MutateBits(zero_u64, 1, 2, true), uint64_t(6));
  EXPECT_EQ(MutateBits(max_u64, 1, 2, true), max_u64);
  EXPECT_EQ(MutateBits(max_u64, 1, 2, false), uint64_t(0xFFFFFFFFFFFFFFF9));
  EXPECT_EQ(MutateBits(zero_u64, 2, 3, true), uint64_t(0x000000000000001C));
  EXPECT_EQ(MutateBits(zero_u64, 2, 35, true), uint64_t(0x0000001FFFFFFFFC));
  EXPECT_EQ(MutateBits(zero_u64, 36, 4, true), uint64_t(0x000000F000000000));
  EXPECT_EQ(MutateBits(max_u64, 36, 4, false), uint64_t(0xFFFFFF0FFFFFFFFF));
}

TEST(BitUtilsTest, SetHighBitsUint32) {
  const uint32_t zero_u32 = 0;
  const uint32_t one_u32 = 1;
  const uint32_t max_u32 = ~zero_u32;

  EXPECT_EQ(SetHighBits(zero_u32, 0), zero_u32);
  EXPECT_EQ(SetHighBits(zero_u32, 1), 0x80000000);
  EXPECT_EQ(SetHighBits(one_u32, 1), 0x80000001);
  EXPECT_EQ(SetHighBits(one_u32, 2), 0xC0000001);
  EXPECT_EQ(SetHighBits(zero_u32, 31), 0xFFFFFFFE);
  EXPECT_EQ(SetHighBits(zero_u32, 32), max_u32);
  EXPECT_EQ(SetHighBits(max_u32, 32), max_u32);
}

TEST(BitUtilsTest, ClearHighBitsUint32) {
  const uint32_t zero_u32 = 0;
  const uint32_t one_u32 = 1;
  const uint32_t max_u32 = ~zero_u32;

  EXPECT_EQ(ClearHighBits(zero_u32, 0), zero_u32);
  EXPECT_EQ(ClearHighBits(zero_u32, 1), zero_u32);
  EXPECT_EQ(ClearHighBits(one_u32, 1), one_u32);
  EXPECT_EQ(ClearHighBits(one_u32, 31), one_u32);
  EXPECT_EQ(ClearHighBits(one_u32, 32), zero_u32);
  EXPECT_EQ(ClearHighBits(max_u32, 0), max_u32);
  EXPECT_EQ(ClearHighBits(max_u32, 1), 0x7FFFFFFF);
  EXPECT_EQ(ClearHighBits(max_u32, 2), 0x3FFFFFFF);
  EXPECT_EQ(ClearHighBits(max_u32, 31), one_u32);
  EXPECT_EQ(ClearHighBits(max_u32, 32), zero_u32);
}

TEST(BitUtilsTest, IsBitSetAtPositionZero) {
  const uint32_t zero_u32 = 0;
  for (size_t i = 0; i != 32; ++i) {
    EXPECT_FALSE(IsBitAtPositionSet(zero_u32, i));
  }

  const uint8_t zero_u8 = 0;
  for (size_t i = 0; i != 8; ++i) {
    EXPECT_FALSE(IsBitAtPositionSet(zero_u8, i));
  }

  const uint64_t zero_u64 = 0;
  for (size_t i = 0; i != 64; ++i) {
    EXPECT_FALSE(IsBitAtPositionSet(zero_u64, i));
  }
}

TEST(BitUtilsTest, IsBitSetAtPositionOne) {
  const uint32_t one_u32 = 1;
  for (size_t i = 0; i != 32; ++i) {
    if (i == 0) {
      EXPECT_TRUE(IsBitAtPositionSet(one_u32, i));
    } else {
      EXPECT_FALSE(IsBitAtPositionSet(one_u32, i));
    }
  }

  const uint32_t two_to_17_u32 = 1 << 17;
  for (size_t i = 0; i != 32; ++i) {
    if (i == 17) {
      EXPECT_TRUE(IsBitAtPositionSet(two_to_17_u32, i));
    } else {
      EXPECT_FALSE(IsBitAtPositionSet(two_to_17_u32, i));
    }
  }

  const uint8_t two_to_4_u8 = 1 << 4;
  for (size_t i = 0; i != 8; ++i) {
    if (i == 4) {
      EXPECT_TRUE(IsBitAtPositionSet(two_to_4_u8, i));
    } else {
      EXPECT_FALSE(IsBitAtPositionSet(two_to_4_u8, i));
    }
  }

  const uint64_t two_to_55_u64 = uint64_t(1) << 55;
  for (size_t i = 0; i != 64; ++i) {
    if (i == 55) {
      EXPECT_TRUE(IsBitAtPositionSet(two_to_55_u64, i));
    } else {
      EXPECT_FALSE(IsBitAtPositionSet(two_to_55_u64, i));
    }
  }
}

TEST(BitUtilsTest, IsBitSetAtPositionAll) {
  const uint32_t max_u32 = ~0;
  for (size_t i = 0; i != 32; ++i) {
    EXPECT_TRUE(IsBitAtPositionSet(max_u32, i));
  }

  const uint32_t max_u8 = ~uint8_t(0);
  for (size_t i = 0; i != 8; ++i) {
    EXPECT_TRUE(IsBitAtPositionSet(max_u8, i));
  }

  const uint64_t max_u64 = ~uint64_t(0);
  for (size_t i = 0; i != 64; ++i) {
    EXPECT_TRUE(IsBitAtPositionSet(max_u64, i));
  }
}
}  // namespace
}  // namespace utils
}  // namespace spvtools
