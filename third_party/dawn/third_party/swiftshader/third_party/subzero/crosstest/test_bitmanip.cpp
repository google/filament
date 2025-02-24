//===- subzero/crosstest/test_bitmanip.cpp - Implementation for tests. ----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This aims to test that all the bit manipulation intrinsics work, via
// cross-testing. This calls wrappers (my_{ctlz,cttz,ctpop} around the
// intrinsics (llvm.{ctlz,cttz,ctpop}.*).
//===----------------------------------------------------------------------===//

#include <stdint.h>

#include <cstdlib>

#include "test_bitmanip.h"

#define X(inst, type)                                                          \
  type test_##inst(type a) { return my_##inst(a); }                            \
  type test_alloca_##inst(type a) {                                            \
    const size_t buf_size = 8;                                                 \
    type buf[buf_size];                                                        \
    for (size_t i = 0; i < buf_size; ++i) {                                    \
      buf[i] = my_##inst(a);                                                   \
    }                                                                          \
    type sum = 0;                                                              \
    for (size_t i = 0; i < buf_size; ++i) {                                    \
      sum += buf[i];                                                           \
    }                                                                          \
    return sum;                                                                \
  }                                                                            \
  type test_const_##inst(type ignored) {                                       \
    return my_##inst(static_cast<type>(0x12340));                              \
  }

FOR_ALL_BMI_OP_TYPES(X)
#undef X

#define X(type, builtin_name)                                                  \
  type test_bswap(type a) { return builtin_name(a); }                          \
  type test_bswap_alloca(type a) {                                             \
    const size_t buf_size = 8;                                                 \
    type buf[buf_size];                                                        \
    for (size_t i = 0; i < buf_size; ++i) {                                    \
      buf[i] = builtin_name(a * i) + builtin_name(a + i);                      \
    }                                                                          \
    type sum = 0;                                                              \
    for (size_t i = 0; i < buf_size; ++i) {                                    \
      sum += buf[i];                                                           \
    }                                                                          \
    return sum;                                                                \
  }
BSWAP_TABLE
#undef X
