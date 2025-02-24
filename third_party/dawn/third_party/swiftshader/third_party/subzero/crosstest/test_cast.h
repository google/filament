//===- subzero/crosstest/test_cast.h - Test prototypes ----------*- C++ -*-===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the function prototypes used for crosstesting cast
// operations.
//
//===----------------------------------------------------------------------===//

// The driver and the test program may be compiled by different
// versions of clang, with different standard libraries that have
// different definitions of int8_t.  Specifically, int8_t may be
// typedef'd as either 'char' or 'signed char', which mangle to
// different strings.  Avoid int8_t and use an explicit myint8_t.
typedef signed char myint8_t;

template <typename FromType, typename ToType> ToType cast(FromType a);
template <typename FromType, typename ToType> ToType castBits(FromType a);

// Targets like MIPS32, pass floating-point arguments in general purpose
// registers when the first argument is passed in a general purpose register.
// Overloaded cast and castBits functions take two extra integer argument to
// check proper conversion of floating-point to/from general purpose registers.
template <typename FromType, typename ToType>
ToType cast(int i, FromType a, int j);
template <typename FromType, typename ToType>
ToType castBits(int i, FromType a, int j);
