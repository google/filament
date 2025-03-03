//===- unittests/Support/EndianTest.cpp - Endian.h tests ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Endian.h"
#include "llvm/Support/DataTypes.h"
#include "gtest/gtest.h"
#include <cstdlib>
#include <ctime>
using namespace llvm;
using namespace support;

#undef max

namespace {

TEST(Endian, Read) {
  // These are 5 bytes so we can be sure at least one of the reads is unaligned.
  unsigned char bigval[] = {0x00, 0x01, 0x02, 0x03, 0x04};
  unsigned char littleval[] = {0x00, 0x04, 0x03, 0x02, 0x01};
  int32_t BigAsHost = 0x00010203;
  EXPECT_EQ(BigAsHost, (endian::read<int32_t, big, unaligned>(bigval)));
  int32_t LittleAsHost = 0x02030400;
  EXPECT_EQ(LittleAsHost,(endian::read<int32_t, little, unaligned>(littleval)));

  EXPECT_EQ((endian::read<int32_t, big, unaligned>(bigval + 1)),
            (endian::read<int32_t, little, unaligned>(littleval + 1)));
}

TEST(Endian, Write) {
  unsigned char data[5];
  endian::write<int32_t, big, unaligned>(data, -1362446643);
  EXPECT_EQ(data[0], 0xAE);
  EXPECT_EQ(data[1], 0xCA);
  EXPECT_EQ(data[2], 0xB6);
  EXPECT_EQ(data[3], 0xCD);
  endian::write<int32_t, big, unaligned>(data + 1, -1362446643);
  EXPECT_EQ(data[1], 0xAE);
  EXPECT_EQ(data[2], 0xCA);
  EXPECT_EQ(data[3], 0xB6);
  EXPECT_EQ(data[4], 0xCD);

  endian::write<int32_t, little, unaligned>(data, -1362446643);
  EXPECT_EQ(data[0], 0xCD);
  EXPECT_EQ(data[1], 0xB6);
  EXPECT_EQ(data[2], 0xCA);
  EXPECT_EQ(data[3], 0xAE);
  endian::write<int32_t, little, unaligned>(data + 1, -1362446643);
  EXPECT_EQ(data[1], 0xCD);
  EXPECT_EQ(data[2], 0xB6);
  EXPECT_EQ(data[3], 0xCA);
  EXPECT_EQ(data[4], 0xAE);
}

TEST(Endian, PackedEndianSpecificIntegral) {
  // These are 5 bytes so we can be sure at least one of the reads is unaligned.
  unsigned char big[] = {0x00, 0x01, 0x02, 0x03, 0x04};
  unsigned char little[] = {0x00, 0x04, 0x03, 0x02, 0x01};
  big32_t    *big_val    =
    reinterpret_cast<big32_t *>(big + 1);
  little32_t *little_val =
    reinterpret_cast<little32_t *>(little + 1);

  EXPECT_EQ(*big_val, *little_val);
}

} // end anon namespace
