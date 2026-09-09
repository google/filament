//===-- String.cpp - SPIR-V Strings -----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/SPIRV/String.h"
#include <assert.h>

namespace clang {
namespace spirv {
namespace string {

/// \brief Reinterprets a given string as sequence of words.
std::vector<uint32_t> encodeSPIRVString(llvm::StringRef strChars) {
  // Initialize all words to 0.
  size_t numChars = strChars.size();
  std::vector<uint32_t> result(numChars / 4 + 1, 0);

  // From the SPIR-V spec, literal string is
  //
  // A nul-terminated stream of characters consuming an integral number of
  // words. The character set is Unicode in the UTF-8 encoding scheme. The UTF-8
  // octets (8-bit bytes) are packed four per word, following the little-endian
  // convention (i.e., the first octet is in the lowest-order 8 bits of the
  // word). The final word contains the string's nul-termination character (0),
  // and all contents past the end of the string in the final word are padded
  // with 0.
  //
  // So the following works on little endian machines.
  char *strDest = reinterpret_cast<char *>(result.data());
  strncpy(strDest, strChars.data(), numChars);
  return result;
}

/// \brief Reinterprets the given vector of 32-bit words as a string.
/// Expectes that the words represent a NULL-terminated string.
/// Assumes Little Endian architecture.
std::string decodeSPIRVString(llvm::ArrayRef<uint32_t> strWords) {
  if (!strWords.empty()) {
    return reinterpret_cast<const char *>(strWords.data());
  }

  return "";
}

} // end namespace string
} // end namespace spirv
} // end namespace clang
