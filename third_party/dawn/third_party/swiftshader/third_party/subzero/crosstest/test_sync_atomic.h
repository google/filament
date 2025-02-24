//===- subzero/crosstest/test_sync_atomic.h - Test prototypes ---*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the function prototypes for cross testing atomic
// intrinsics.
//
//===----------------------------------------------------------------------===//

#include "test_sync_atomic.def"

#define X(inst, type)                                                          \
  type test_##inst(bool fetch_first, volatile type *ptr, type a);              \
  type test_alloca_##inst(bool fetch, volatile type *ptr, type a);             \
  type test_const_##inst(bool fetch, volatile type *ptr, type ignored);

FOR_ALL_RMWOP_TYPES(X)
#undef X

#define X(type)                                                                \
  type test_val_cmp_swap(volatile type *ptr, type oldval, type newval);        \
  type test_val_cmp_swap_loop(volatile type *ptr, type oldval, type newval);

ATOMIC_TYPE_TABLE
#undef X
