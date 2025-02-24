//===- subzero/crosstest/test_sync_atomic.cpp - Implementation for tests --===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This aims to test that all the atomic RMW instructions and compare and swap
// work across the allowed atomic types. This uses the __sync_* builtins
// to test the atomic operations.
//
//===----------------------------------------------------------------------===//

#include <stdint.h>

#include <cstdlib>

#include "test_sync_atomic.h"

#define X(inst, type)                                                          \
  type test_##inst(bool fetch_first, volatile type *ptr, type a) {             \
    if (fetch_first) {                                                         \
      return __sync_fetch_and_##inst(ptr, a);                                  \
    } else {                                                                   \
      return __sync_##inst##_and_fetch(ptr, a);                                \
    }                                                                          \
  }                                                                            \
  type test_alloca_##inst(bool fetch, volatile type *ptr, type a) {            \
    const size_t buf_size = 8;                                                 \
    type buf[buf_size];                                                        \
    for (size_t i = 0; i < buf_size; ++i) {                                    \
      if (fetch) {                                                             \
        buf[i] = __sync_fetch_and_##inst(ptr, a);                              \
      } else {                                                                 \
        buf[i] = __sync_##inst##_and_fetch(ptr, a);                            \
      }                                                                        \
    }                                                                          \
    type sum = 0;                                                              \
    for (size_t i = 0; i < buf_size; ++i) {                                    \
      sum += buf[i];                                                           \
    }                                                                          \
    return sum;                                                                \
  }                                                                            \
  type test_const_##inst(bool fetch, volatile type *ptr, type ign) {           \
    if (fetch) {                                                               \
      return __sync_fetch_and_##inst(ptr, 42);                                 \
    } else {                                                                   \
      const type value = static_cast<type>(0xaaaaaaaaaaaaaaaaull);             \
      return __sync_##inst##_and_fetch(ptr, value);                            \
    }                                                                          \
  }

FOR_ALL_RMWOP_TYPES(X)
#undef X

#define X(type)                                                                \
  type test_val_cmp_swap(volatile type *ptr, type oldval, type newval) {       \
    return __sync_val_compare_and_swap(ptr, oldval, newval);                   \
  }                                                                            \
  type test_val_cmp_swap_loop(volatile type *ptr, type oldval, type newval) {  \
    type prev;                                                                 \
    type succeeded_first_try = 1;                                              \
    while (1) {                                                                \
      prev = __sync_val_compare_and_swap(ptr, oldval, newval);                 \
      if (prev == oldval)                                                      \
        break;                                                                 \
      succeeded_first_try = 0;                                                 \
      oldval = prev;                                                           \
    }                                                                          \
    return succeeded_first_try;                                                \
  }

ATOMIC_TYPE_TABLE
#undef X
