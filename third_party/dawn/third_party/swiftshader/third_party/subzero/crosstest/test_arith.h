//===- subzero/crosstest/test_arith.h - Test prototypes ---------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the function prototypes used for crosstesting arithmetic
// operations.
//
//===----------------------------------------------------------------------===//

#include "test_arith.def"
#include "xdefs.h"
#include <stdint.h>

#include "vectors.h"

#define X(inst, op, isdiv, isshift)                                            \
  bool test##inst(bool a, bool b);                                             \
  uint8_t test##inst(uint8_t a, uint8_t b);                                    \
  uint16_t test##inst(uint16_t a, uint16_t b);                                 \
  uint32_t test##inst(uint32_t a, uint32_t b);                                 \
  uint64 test##inst(uint64 a, uint64 b);                                       \
  v4ui32 test##inst(v4ui32 a, v4ui32 b);                                       \
  v8ui16 test##inst(v8ui16 a, v8ui16 b);                                       \
  v16ui8 test##inst(v16ui8 a, v16ui8 b);
UINTOP_TABLE
#undef X

#define X(inst, op, isdiv, isshift)                                            \
  bool test##inst(bool a, bool b);                                             \
  myint8_t test##inst(myint8_t a, myint8_t b);                                 \
  int16_t test##inst(int16_t a, int16_t b);                                    \
  int32_t test##inst(int32_t a, int32_t b);                                    \
  int64 test##inst(int64 a, int64 b);                                          \
  v4si32 test##inst(v4si32 a, v4si32 b);                                       \
  v8si16 test##inst(v8si16 a, v8si16 b);                                       \
  v16si8 test##inst(v16si8 a, v16si8 b);
SINTOP_TABLE
#undef X

float myFrem(float a, float b);
double myFrem(double a, double b);
v4f32 myFrem(v4f32 a, v4f32 b);

#define X(inst, op, func)                                                      \
  float test##inst(float a, float b);                                          \
  double test##inst(double a, double b);                                       \
  v4f32 test##inst(v4f32 a, v4f32 b);
FPOP_TABLE
#undef X

float mySqrt(float a);
double mySqrt(double a);
// mySqrt for v4f32 is currently unsupported.

float myFabs(float a);
double myFabs(double a);
v4f32 myFabs(v4f32 a);

#define X(mult_by)                                                             \
  bool testMultiplyBy##mult_by(bool a, bool);                                  \
  bool testMultiplyByNeg##mult_by(bool a, bool);                               \
  uint8_t testMultiplyBy##mult_by(uint8_t a, uint8_t);                         \
  uint8_t testMultiplyByNeg##mult_by(uint8_t a, uint8_t);                      \
  uint16_t testMultiplyBy##mult_by(uint16_t a, uint16_t);                      \
  uint16_t testMultiplyByNeg##mult_by(uint16_t a, uint16_t);                   \
  uint32_t testMultiplyBy##mult_by(uint32_t a, uint32_t);                      \
  uint32_t testMultiplyByNeg##mult_by(uint32_t a, uint32_t);                   \
  uint64_t testMultiplyBy##mult_by(uint64_t a, uint64_t);                      \
  uint64_t testMultiplyByNeg##mult_by(uint64_t a, uint64_t);
MULIMM_TABLE
#undef X
