//===- subzero/crosstest/test_global.h - Test prototypes --------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the function prototypes used for crosstesting global
// variable access operations.
//
//===----------------------------------------------------------------------===//

// The driver and the test program may be compiled by different
// versions of clang, with different standard libraries that have
// different definitions of int8_t.  Specifically, int8_t may be
// typedef'd as either 'char' or 'signed char', which mangle to
// different strings.  Avoid int8_t and use an explicit myint8_t.
typedef signed char myint8_t;

size_t getNumArrays();
const uint8_t *getArray(size_t WhichArray, size_t &Len);
