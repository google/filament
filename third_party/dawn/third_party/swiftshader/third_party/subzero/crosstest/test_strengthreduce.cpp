//===- subzero/crosstest/test_strengthreduce.cpp - Strength reduction -----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation for crosstesting strength reduction.
//
//===----------------------------------------------------------------------===//

#include "test_strengthreduce.h"

// TODO(stichnot): Extend to i16 and i8 types, and also test the
// commutativity transformations.  This may require hand-generating
// .ll files, because of C/C++ integer promotion rules for arithmetic,
// and because clang prefers to do its own commutativity
// transformation.

#define X(constant, suffix)                                                    \
  uint32_t multiplyByConst##suffix(uint32_t Val) {                             \
    return Val * (uint32_t)constant;                                           \
  }                                                                            \
  int32_t multiplyByConst##suffix(int32_t Val) {                               \
    return Val * (int32_t)constant;                                            \
  }
CONST_TABLE
#undef X
