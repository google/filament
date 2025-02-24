//===- subzero/crosstest/test_stacksave.c - Implementation for tests ------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This aims to test that C99's VLAs (which use stacksave/stackrestore
// intrinsics) work fine.
//
//===----------------------------------------------------------------------===//

#include <stdint.h>

#include "test_stacksave.h"
DECLARE_TESTS()

/* NOTE: This has 0 stacksaves, because the vla isn't in a loop,
 * so the vla can just be freed by the epilogue.
 */
uint32_t test_basic_vla(uint32_t size, uint32_t start, uint32_t inc) {
  uint32_t vla[size];
  uint32_t mid = start + ((size - start) / 2);
  for (uint32_t i = start; i < size; ++i) {
    vla[i] = i + inc;
  }
  return (vla[start] << 2) + (vla[mid] << 1) + vla[size - 1];
}

static uint32_t __attribute__((noinline)) foo(uint32_t x) { return x * x; }

/* NOTE: This has 1 stacksave, because the vla is in a loop and should
 * be freed before the next iteration.
 */
uint32_t test_vla_in_loop(uint32_t size, uint32_t start, uint32_t inc) {
  uint32_t sum = 0;
  for (uint32_t i = start; i < size; ++i) {
    uint32_t size1 = size - i;
    uint32_t vla[size1];
    for (uint32_t j = 0; j < size1; ++j) {
      /* Adjust stack again with a function call. */
      vla[j] = foo(start * j + inc);
    }
    for (uint32_t j = 0; j < size1; ++j) {
      sum += vla[j];
    }
  }
  return sum;
}

uint32_t test_two_vlas_in_loops(uint32_t size, uint32_t start, uint32_t inc) {
  uint32_t sum = 0;
  for (uint32_t i = start; i < size; ++i) {
    uint32_t size1 = size - i;
    uint32_t vla1[size1];
    for (uint32_t j = 0; j < size1; ++j) {
      uint32_t size2 = size - j;
      uint32_t start2 = 0;
      uint32_t mid2 = size2 / 2;
      uint32_t vla2[size2];
      for (uint32_t k = start2; k < size2; ++k) {
        /* Adjust stack again with a function call. */
        vla2[k] = foo(start * k + inc);
      }
      vla1[j] = (vla2[start2] << 2) + (vla2[mid2] << 1) + vla2[size2 - 1];
    }
    for (uint32_t j = 0; j < size1; ++j) {
      sum += vla1[j];
    }
  }
  return sum;
}
