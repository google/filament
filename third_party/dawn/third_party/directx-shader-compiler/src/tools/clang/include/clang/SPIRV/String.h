//===-- String.h - SPIR-V Strings -------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_STRING_H
#define LLVM_CLANG_SPIRV_STRING_H

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"

#include <streambuf>
#include <string>
#include <vector>

namespace clang {
namespace spirv {
namespace string {

/// \brief Reinterprets a given string as sequence of words. It follows the
/// SPIR-V string encoding requirements.
std::vector<uint32_t> encodeSPIRVString(llvm::StringRef strChars);

/// \brief Reinterprets the given vector of 32-bit words as a string.
/// Expectes that the words represent a NULL-terminated string.
/// It follows the SPIR-V string encoding requirements.
std::string decodeSPIRVString(llvm::ArrayRef<uint32_t> strWords);

/// \brief Stream buffer implementation that writes to `llvm::raw_ostream`.
/// Intended to be used with APIs that write to `std::ostream`.
///
/// Sample:
/// ```
/// RawOstreamBuf buf(llvm::errs());
/// std::ostream os(&buf);
/// some_print(os);
/// ```
class RawOstreamBuf : public std::streambuf {
public:
  RawOstreamBuf(llvm::raw_ostream &os) : os(os) {}
  using char_type = std::streambuf::char_type;
  using int_type = std::streambuf::int_type;

protected:
  std::streamsize xsputn(const char_type *s, std::streamsize count) override {
    os << llvm::StringRef(s, count);
    return count;
  }

  int_type overflow(int_type c) override {
    os << char_type(c);
    return 0;
  }

private:
  llvm::raw_ostream &os;
};

} // end namespace string
} // end namespace spirv
} // end namespace clang

#endif