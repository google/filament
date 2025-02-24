//===- subzero/crosstest/test_stacksave.h - Test prototypes -----*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the function prototypes for cross testing
// stacksave and stackrestore intrinsics.
//
//===----------------------------------------------------------------------===//

#ifndef TEST_STACKSAVE_H
#define TEST_STACKSAVE_H

#define DECLARE_TESTS(PREFIX)                                                  \
  uint32_t PREFIX##test_basic_vla(uint32_t size, uint32_t start,               \
                                  uint32_t inc);                               \
  uint32_t PREFIX##test_vla_in_loop(uint32_t size, uint32_t start,             \
                                    uint32_t inc);                             \
  uint32_t PREFIX##test_two_vlas_in_loops(uint32_t size, uint32_t start,       \
                                          uint32_t inc);

#endif
