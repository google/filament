//===- subzero/crosstest/test_strengthreduce.h - Prototypes ---*- C++ -*---===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the function prototypes used for crosstesting strength
// reduction.
//
//===----------------------------------------------------------------------===//

#include <stdint.h>

#include "test_strengthreduce.def"

#define X(constant, suffix)                                                    \
  uint32_t multiplyByConst##suffix(uint32_t val);                              \
  int32_t multiplyByConst##suffix(int32_t val);
CONST_TABLE
#undef X
