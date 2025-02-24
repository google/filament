//===- subzero/crosstest/test_cast.cpp - Cast operator tests --------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Implementation for crosstesting cast operations.
//
//===----------------------------------------------------------------------===//

// This aims to test all the conversion bitcode instructions across
// all PNaCl primitive data types.

#include "test_cast.h"
#include "xdefs.h"
#include <stdint.h>

template <typename FromType, typename ToType>
ToType __attribute__((noinline)) cast(FromType a) {
  return (ToType)a;
}

template <typename FromType, typename ToType>
ToType __attribute__((noinline)) castBits(FromType a) {
  return *(ToType *)&a;
}

template <typename FromType, typename ToType>
ToType __attribute__((noinline)) cast(int i, FromType a, int j) {
  (void)i;
  (void)j;
  return (ToType)a;
}

template <typename FromType, typename ToType>
ToType __attribute__((noinline)) castBits(int i, FromType a, int j) {
  (void)i;
  (void)j;
  return *(ToType *)&a;
}

// The purpose of the following sets of templates is to force
// cast<A,B>() to be instantiated in the resulting bitcode file for
// all <A,B>, so that they can be called from the driver.
template <typename ToType> class Caster {
  static ToType f(bool a) { return cast<bool, ToType>(a); }
  static ToType f(myint8_t a) { return cast<myint8_t, ToType>(a); }
  static ToType f(uint8_t a) { return cast<uint8_t, ToType>(a); }
  static ToType f(int16_t a) { return cast<int16_t, ToType>(a); }
  static ToType f(uint16_t a) { return cast<uint16_t, ToType>(a); }
  static ToType f(int32_t a) { return cast<int32_t, ToType>(a); }
  static ToType f(uint32_t a) { return cast<uint32_t, ToType>(a); }
  static ToType f(int64 a) { return cast<int64, ToType>(a); }
  static ToType f(uint64 a) { return cast<uint64, ToType>(a); }
  static ToType f(float a) { return cast<float, ToType>(a); }
  static ToType f(double a) { return cast<double, ToType>(a); }
  static ToType f(int i, bool a) { return cast<bool, ToType>(i, a, i); }
  static ToType f(int i, myint8_t a) { return cast<myint8_t, ToType>(i, a, i); }
  static ToType f(int i, uint8_t a) { return cast<uint8_t, ToType>(i, a, i); }
  static ToType f(int i, int16_t a) { return cast<int16_t, ToType>(i, a, i); }
  static ToType f(int i, uint16_t a) { return cast<uint16_t, ToType>(i, a, i); }
  static ToType f(int i, int32_t a) { return cast<int32_t, ToType>(i, a, i); }
  static ToType f(int i, uint32_t a) { return cast<uint32_t, ToType>(i, a, i); }
  static ToType f(int i, int64 a) { return cast<int64, ToType>(i, a, i); }
  static ToType f(int i, uint64 a) { return cast<uint64, ToType>(i, a, i); }
  static ToType f(int i, float a) { return cast<float, ToType>(i, a, i); }
  static ToType f(int i, double a) { return cast<double, ToType>(i, a, i); }
};

// Comment out the definition of Caster<bool> because clang compiles
// casts to bool using icmp instead of the desired cast instruction.
// The corrected definitions are in test_cast_to_u1.ll.

// template class Caster<bool>;

template class Caster<myint8_t>;
template class Caster<uint8_t>;
template class Caster<int16_t>;
template class Caster<uint16_t>;
template class Caster<int32_t>;
template class Caster<uint32_t>;
template class Caster<int64>;
template class Caster<uint64>;
template class Caster<float>;
template class Caster<double>;

// This function definition forces castBits<A,B>() to be instantiated
// in the resulting bitcode file for the 4 relevant <A,B>
// combinations, so that they can be called from the driver.
double makeBitCasters() {
  double Result = 0;
  Result += castBits<uint32_t, float>(0);
  Result += castBits<uint64, double>(0);
  Result += castBits<float, uint32_t>(0);
  Result += castBits<double, uint64>(0);
  Result += castBits<uint32_t, float>(1, 0, 2);
  Result += castBits<uint64, double>(1, 0, 2);
  Result += castBits<float, uint32_t>(1, 0, 2);
  Result += castBits<double, uint64>(1, 0, 2);
  return Result;
}
