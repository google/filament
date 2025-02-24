//===- subzero/crosstest/test_icmp.cpp - Implementation for tests ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This aims to test the icmp bitcode instruction across all PNaCl primitive
// and SIMD integer types.
//
//===----------------------------------------------------------------------===//

#include <stdint.h>

#include "test_icmp.h"
#include "xdefs.h"

#define X(cmp, op)                                                             \
  bool icmp##cmp(uint8_t a, uint8_t b) { return a op b; }                      \
  bool icmp##cmp(uint16_t a, uint16_t b) { return a op b; }                    \
  bool icmp##cmp(uint32_t a, uint32_t b) { return a op b; }                    \
  bool icmp##cmp(uint64 a, uint64 b) { return a op b; }                        \
  v4ui32 icmp##cmp(v4ui32 a, v4ui32 b) { return a op b; }                      \
  v8ui16 icmp##cmp(v8ui16 a, v8ui16 b) { return a op b; }                      \
  v16ui8 icmp##cmp(v16ui8 a, v16ui8 b) { return a op b; }                      \
  bool icmp_zero##cmp(uint8_t a) { return a op 0; }                            \
  bool icmp_zero##cmp(uint16_t a) { return a op 0; }                           \
  bool icmp_zero##cmp(uint32_t a) { return a op 0; }                           \
  bool icmp_zero##cmp(uint64 a) { return a op 0; }
ICMP_U_TABLE
#undef X

#define X(cmp, op)                                                             \
  bool icmp##cmp(myint8_t a, myint8_t b) { return a op b; }                    \
  bool icmp##cmp(int16_t a, int16_t b) { return a op b; }                      \
  bool icmp##cmp(int32_t a, int32_t b) { return a op b; }                      \
  bool icmp##cmp(int64 a, int64 b) { return a op b; }                          \
  v4si32 icmp##cmp(v4si32 a, v4si32 b) { return a op b; }                      \
  v8si16 icmp##cmp(v8si16 a, v8si16 b) { return a op b; }                      \
  v16si8 icmp##cmp(v16si8 a, v16si8 b) { return a op b; }                      \
  bool icmp_zero##cmp(myint8_t a) { return a op 0; }                           \
  bool icmp_zero##cmp(int16_t a) { return a op 0; }                            \
  bool icmp_zero##cmp(int32_t a) { return a op 0; }                            \
  bool icmp_zero##cmp(int64 a) { return a op 0; }
ICMP_S_TABLE
#undef X
