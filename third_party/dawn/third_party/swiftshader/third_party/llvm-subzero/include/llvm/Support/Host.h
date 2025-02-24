//===- llvm/Support/Host.h - Host machine characteristics --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Methods for querying the nature of the host machine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_HOST_H
#define LLVM_SUPPORT_HOST_H

#include "llvm/ADT/StringMap.h"

#if defined(__linux__) || defined(__GNU__) || defined(__HAIKU__)
#include <endian.h>
#elif defined(_AIX)
#include <sys/machine.h>
#else
#if !defined(BYTE_ORDER) && !defined(LLVM_ON_WIN32)
#include <machine/endian.h>
#endif
#endif

#include <string>

namespace llvm {
namespace sys {

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
  static const bool IsBigEndianHost = true;
#else
  static const bool IsBigEndianHost = false;
#endif

  static const bool IsLittleEndianHost = !IsBigEndianHost;
}
}

#endif
