//===- unittests/DxilHash/DxilHashTest.cpp ---- Run DxilHash tests --------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// DxilHashing.h unit tests.
//
//===----------------------------------------------------------------------===//

using BYTE = unsigned char;
using UINT32 = unsigned int;

#include "dxc/DxilHash/DxilHash.h"
#include "gtest/gtest.h"

namespace {

struct OutputHash {
  BYTE Data[DXIL_CONTAINER_HASH_SIZE];
  bool equal(UINT32 A, UINT32 B, UINT32 C, UINT32 D) {
    UINT32 *DataPtr = (UINT32 *)Data;
    return DataPtr[0] == A && DataPtr[1] == B && DataPtr[2] == C &&
           DataPtr[3] == D;
  }
};

bool operator==(const OutputHash &A, const OutputHash &B) {
  return memcmp(&A, &B, sizeof(OutputHash)) == 0;
}
bool operator!=(const OutputHash &A, const OutputHash &B) {
  return memcmp(&A, &B, sizeof(OutputHash)) != 0;
}

template <typename T> OutputHash hash_value(T Input) {
  OutputHash O;
  ComputeHashRetail((const BYTE *)&Input, sizeof(T), (BYTE *)&O);
  return O;
}

template <> OutputHash hash_value<std::string>(std::string S) {
  OutputHash O;
  ComputeHashRetail((const BYTE *)S.data(), S.size(), (BYTE *)&O);
  return O;
}

enum TestEnumeration { TE_Foo = 42, TE_Bar = 43 };

TEST(DxilHashTest, HashValueBasicTest) {
  int x = 42, y = 43, c = 'x';
  void *p = nullptr;
  uint64_t i = 71;
  const unsigned ci = 71;
  volatile int vi = 71;
  const volatile int cvi = 71;
  uintptr_t addr = reinterpret_cast<uintptr_t>(&y);
  EXPECT_EQ(hash_value(42), hash_value(x));
  EXPECT_EQ(hash_value(42), hash_value(TE_Foo));
  EXPECT_NE(hash_value(42), hash_value(y));
  EXPECT_NE(hash_value(42), hash_value(TE_Bar));
  EXPECT_NE(hash_value(42), hash_value(p));
  EXPECT_EQ(hash_value(71), hash_value(ci));
  EXPECT_EQ(hash_value(71), hash_value(vi));
  EXPECT_EQ(hash_value(71), hash_value(cvi));
  EXPECT_EQ(hash_value(addr), hash_value(&y));
  // Miss match for type mismatch.
  EXPECT_NE(hash_value(71), hash_value(i));
  EXPECT_NE(hash_value(c), hash_value('x'));
  EXPECT_NE(hash_value('4'), hash_value('0' + 4));

  std::string strA = "42";
  std::string strB = "";
  std::string strC = "42";

  EXPECT_NE(hash_value(strA), hash_value(strB));
  EXPECT_EQ(hash_value(strA), hash_value(strC));
}

TEST(DxilHashTest, FixHashValueTest) {
  std::string Data = "";
  EXPECT_EQ(
      hash_value(Data).equal(0xf6600d14, 0xbae275b7, 0xd4be4a4e, 0xa1e9b201),
      true);

  std::string Data2 = "abcdefghijklmnopqrstuvwxyzabcdef"
                      "abcdefghijklmnopqrstuvwxyzghijkl"
                      "abcdefghijklmnopqrstuvwxyzmnopqr"
                      "abcdefghijklmnopqrstuvwxyzstuvwx"
                      "abcdefghijklmnopqrstuvwxyzyzabcd";
  EXPECT_EQ(
      hash_value(Data2).equal(0x00830fe6, 0xd8d8a035, 0xe6d62794, 0x016629df),
      true);

  std::string Data3 = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                      "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  EXPECT_EQ(
      hash_value(Data3).equal(0xa5e5a0bb, 0xd96dd9f8, 0x0b4b7191, 0xd63aa54a),
      true);

  std::string Data4 = "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
                      "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
                      "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
                      "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"
                      "zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz";
  EXPECT_EQ(
      hash_value(Data4).equal(0x58ed1b7a, 0x90ede58b, 0xf9ad857c, 0x5440e613),
      true);

  std::string Data5 = "abababababababababababababababab"
                      "abababababababababababababababab"
                      "abababababababababababababababab"
                      "abababababababababababababababab"
                      "abababababababababababababababab";
  EXPECT_EQ(
      hash_value(Data5).equal(0xf4fc06fc, 0x0bbd9ef7, 0x765ae0f7, 0x52a55925),
      true);
}

} // namespace
