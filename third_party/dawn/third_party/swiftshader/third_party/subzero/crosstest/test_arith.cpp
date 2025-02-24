//===- subzero/crosstest/test_arith.cpp - Arithmetic operator tests -------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation for crosstesting arithmetic operations.
//
//===----------------------------------------------------------------------===//

// This aims to test all the arithmetic bitcode instructions across
// all PNaCl primitive data types.

#include <stdint.h>

#include "test_arith.h"
#include "xdefs.h"

#if 0
// The following is commented out, and instead, a python script auto-generates a
// .ll file with the equivalent functionality.

#define X(inst, op, isdiv, isshift)                                            \
  bool test##inst(bool a, bool b) { return a op b; }                           \
  uint8_t test##inst(uint8_t a, uint8_t b) { return a op b; }                  \
  uint16_t test##inst(uint16_t a, uint16_t b) { return a op b; }               \
  uint32_t test##inst(uint32_t a, uint32_t b) { return a op b; }               \
  uint64 test##inst(uint64 a, uint64 b) { return a op b; }                     \
  v4ui32 test##inst(v4ui32 a, v4ui32 b) { return a op b; }                     \
  v8ui16 test##inst(v8ui16 a, v8ui16 b) { return a op b; }                     \
  v16ui8 test##inst(v16ui8 a, v16ui8 b) { return a op b; }
UINTOP_TABLE
#undef X

#define X(inst, op, isdiv, isshift)                                            \
  bool test##inst(bool a, bool b) { return a op b; }                           \
  myint8_t test##inst(myint8_t a, myint8_t b) { return a op b; }               \
  int16_t test##inst(int16_t a, int16_t b) { return a op b; }                  \
  int32_t test##inst(int32_t a, int32_t b) { return a op b; }                  \
  int64 test##inst(int64 a, int64 b) { return a op b; }                        \
  v4si32 test##inst(v4si32 a, v4si32 b) { return a op b; }                     \
  v8si16 test##inst(v8si16 a, v8si16 b) { return a op b; }                     \
  v16si8 test##inst(v16si8 a, v16si8 b) { return a op b; }
SINTOP_TABLE
#undef X

#define X(inst, op, func)                                                      \
  float test##inst(float a, float b) { return func(a op b); }                  \
  double test##inst(double a, double b) { return func(a op b); }               \
  v4f32 test##inst(v4f32 a, v4f32 b) { return func(a op b); }
FPOP_TABLE
#undef X

#endif // 0

#define X(mult_by)                                                             \
  bool testMultiplyBy##mult_by(bool a, bool /*unused*/) {                      \
    return a * (mult_by);                                                      \
  }                                                                            \
  bool testMultiplyByNeg##mult_by(bool a, bool /*unused*/) {                   \
    return a * (-(mult_by));                                                   \
  }                                                                            \
  uint8_t testMultiplyBy##mult_by(uint8_t a, uint8_t /*unused*/) {             \
    return a * (mult_by);                                                      \
  }                                                                            \
  uint8_t testMultiplyByNeg##mult_by(uint8_t a, uint8_t /*unused*/) {          \
    return a * (-(mult_by));                                                   \
  }                                                                            \
  uint16_t testMultiplyBy##mult_by(uint16_t a, uint16_t /*unused*/) {          \
    return a * (mult_by);                                                      \
  }                                                                            \
  uint16_t testMultiplyByNeg##mult_by(uint16_t a, uint16_t /*unused*/) {       \
    return a * (-(mult_by));                                                   \
  }                                                                            \
  uint32_t testMultiplyBy##mult_by(uint32_t a, uint32_t /*unused*/) {          \
    return a * (mult_by);                                                      \
  }                                                                            \
  uint32_t testMultiplyByNeg##mult_by(uint32_t a, uint32_t /*unused*/) {       \
    return a * (-(mult_by));                                                   \
  }                                                                            \
  uint64_t testMultiplyBy##mult_by(uint64_t a, uint64_t /*unused*/) {          \
    return a * (mult_by);                                                      \
  }                                                                            \
  uint64_t testMultiplyByNeg##mult_by(uint64_t a, uint64_t /*unused*/) {       \
    return a * (-(mult_by));                                                   \
  }
MULIMM_TABLE
#undef X
