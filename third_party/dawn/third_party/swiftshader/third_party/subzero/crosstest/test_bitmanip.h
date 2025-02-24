//===- subzero/crosstest/test_bitmanip.h - Test prototypes ---*- C++ -*----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the function prototypes for cross testing bit
// manipulation intrinsics.
//
//===----------------------------------------------------------------------===//

#include "test_bitmanip.def"

#define X(inst, type)                                                          \
  type test_##inst(type a);                                                    \
  type test_alloca_##inst(type a);                                             \
  type test_const_##inst(type ignored);                                        \
  type my_##inst(type a);

FOR_ALL_BMI_OP_TYPES(X)
#undef X

#define X(type, builtin_name)                                                  \
  type test_bswap(type);                                                       \
  type test_bswap_alloca(type);
BSWAP_TABLE
#undef X
