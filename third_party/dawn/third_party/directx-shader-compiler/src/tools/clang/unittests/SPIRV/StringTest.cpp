//===- unittests/SPIRV/StringTest.cpp ---- SPIR-V String tests ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/SPIRV/String.h"
#include "llvm/Support/raw_ostream.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <ostream>

namespace {

using namespace clang::spirv;
using ::testing::ElementsAre;

TEST(String, EncodeEmptyString) {
  std::string str = "";
  std::vector<uint32_t> words = string::encodeSPIRVString(str);
  EXPECT_THAT(words, ElementsAre(0u));
}
TEST(String, EncodeOneCharString) {
  std::string str = "m";
  std::vector<uint32_t> words = string::encodeSPIRVString(str);
  EXPECT_THAT(words, ElementsAre(109u));
}
TEST(String, EncodeTwoCharString) {
  std::string str = "ma";
  std::vector<uint32_t> words = string::encodeSPIRVString(str);
  EXPECT_THAT(words, ElementsAre(24941u));
}
TEST(String, EncodeThreeCharString) {
  std::string str = "mai";
  std::vector<uint32_t> words = string::encodeSPIRVString(str);
  EXPECT_THAT(words, ElementsAre(6906221u));
}
TEST(String, EncodeFourCharString) {
  std::string str = "main";
  std::vector<uint32_t> words = string::encodeSPIRVString(str);
  EXPECT_THAT(words, ElementsAre(1852399981u, 0u));
}
TEST(String, EncodeString) {
  // Bin  01110100   01110011    01100101    01010100 = unsigned(1,953,719,636)
  // Hex     74         73          65          54
  //          t          s           e           T
  // Bin  01101001   01110010    01110100    01010011 =  unsigned(1,769,108,563)
  // Hex     69         72          74          53
  //          i          r           t           S
  // Bin  00000000   00000000    01100111    01101110 =  unsigned(26,478)
  // Hex      0          0          67          6E
  //          \0         \0          g           n
  std::string str = "TestString";
  std::vector<uint32_t> words = string::encodeSPIRVString(str);
  EXPECT_THAT(words, ElementsAre(1953719636, 1769108563, 26478));
}
TEST(String, DecodeString) {
  // Bin  01110100   01110011    01100101    01010100 = unsigned(1,953,719,636)
  // Hex     74         73          65          54
  //          t          s           e           T
  // Bin  01101001   01110010    01110100    01010011 =  unsigned(1,769,108,563)
  // Hex     69         72          74          53
  //          i          r           t           S
  // Bin  00000000   00000000    01100111    01101110 =  unsigned(26,478)
  // Hex      0          0          67          6E
  //          \0         \0          g           n
  std::vector<uint32_t> words = {1953719636, 1769108563, 26478};
  std::string str = string::decodeSPIRVString(words);
  EXPECT_EQ(str, "TestString");
}
TEST(String, EncodeAndDecodeString) {
  std::string str = "TestString";
  // Convert to vector
  std::vector<uint32_t> words = string::encodeSPIRVString(str);

  // Convert back to string
  std::string result = string::decodeSPIRVString(words);

  EXPECT_EQ(str, result);
}
// TODO: Add more ModuleBuilder tests

TEST(String, RawOstreamBuf) {
  // Set up the following output stream structure:
  //  os -> buf -> rawOS -> underlyingBuffer
  //
  // Check that the contents written to `os` appear in `underlyingBuffer`.
  std::string underlingBuffer;
  llvm::raw_string_ostream rawOS(underlingBuffer);
  string::RawOstreamBuf buf(rawOS);
  std::ostream os(&buf);

  // Flushes both buffers.
  auto flush = [&rawOS, &os] {
    os.flush();
    rawOS.flush();
  };

  EXPECT_EQ(underlingBuffer, "");
  os << "test";
  flush();
  EXPECT_EQ(underlingBuffer, "test");
  os << 13 << 37 << "\n";
  flush();
  EXPECT_EQ(underlingBuffer, "test1337\n");
  os << ' ';
  flush();
  EXPECT_EQ(underlingBuffer, "test1337\n ");
}

} // anonymous namespace
