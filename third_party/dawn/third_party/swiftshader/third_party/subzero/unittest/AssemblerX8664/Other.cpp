//===- subzero/unittest/AssemblerX8664/GPRArith.cpp -----------------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "AssemblerX8664/TestUtil.h"

namespace Ice {
namespace X8664 {
namespace Test {
namespace {

TEST_F(AssemblerX8664LowLevelTest, Nop) {
#define TestImpl(Size, ...)                                                    \
  do {                                                                         \
    static constexpr char TestString[] = "(" #Size ", " #__VA_ARGS__ ")";      \
    __ nop(Size);                                                              \
    ASSERT_EQ(Size##u, codeBytesSize()) << TestString;                         \
    ASSERT_TRUE(verifyBytes<Size>(codeBytes(), __VA_ARGS__)) << TestString;    \
    reset();                                                                   \
  } while (0);

  TestImpl(1, 0x90);
  TestImpl(2, 0x66, 0x90);
  TestImpl(3, 0x0F, 0x1F, 0x00);
  TestImpl(4, 0x0F, 0x1F, 0x40, 0x00);
  TestImpl(5, 0x0F, 0x1F, 0x44, 0x00, 0x00);
  TestImpl(6, 0x66, 0x0F, 0x1F, 0x44, 0x00, 0x00);
  TestImpl(7, 0x0F, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00);
  TestImpl(8, 0x0F, 0x1F, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00);

#undef TestImpl
}

TEST_F(AssemblerX8664LowLevelTest, Int3) {
  __ int3();
  static constexpr uint32_t ByteCount = 1;
  ASSERT_EQ(ByteCount, codeBytesSize());
  verifyBytes<ByteCount>(codeBytes(), 0xCC);
}

TEST_F(AssemblerX8664LowLevelTest, Hlt) {
  __ hlt();
  static constexpr uint32_t ByteCount = 1;
  ASSERT_EQ(ByteCount, codeBytesSize());
  verifyBytes<ByteCount>(codeBytes(), 0xF4);
}

TEST_F(AssemblerX8664LowLevelTest, Ud2) {
  __ ud2();
  static constexpr uint32_t ByteCount = 2;
  ASSERT_EQ(ByteCount, codeBytesSize());
  verifyBytes<ByteCount>(codeBytes(), 0x0F, 0x0B);
}

} // end of anonymous namespace
} // end of namespace Test
} // end of namespace X8664
} // end of namespace Ice
