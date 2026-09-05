//===-- BitwiseCast.h - Bitwise cast ----------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_BITWISECAST_H
#define LLVM_CLANG_SPIRV_BITWISECAST_H

#include <cstring>

namespace clang {
namespace spirv {
namespace cast {

/// \brief Performs bitwise copy of source to the destination type Dest.
template <typename Dest, typename Src> Dest BitwiseCast(Src source) {
  Dest dest;
  static_assert(sizeof(source) == sizeof(dest),
                "BitwiseCast: source and destination must have the same size.");
  std::memcpy(&dest, &source, sizeof(dest));
  return dest;
}

} // end namespace cast
} // end namespace spirv
} // end namespace clang

#endif
