//===- subzero/crosstest/test_select_main.cpp - Driver for tests ----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for crosstesting the select bitcode instruction
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_select.ll  --driver=test_select_main.cpp \
   --prefix=Subzero_ --output=test_select */

#include <cfloat>
#include <cstring>
#include <iostream>

#include "test_arith.def"
#include "test_select.h"

namespace Subzero_ {
#include "test_select.h"
}

#include "insertelement.h"

static const size_t MaxTestsPerFunc = 100000;

template <typename T, typename TI1>
void testSelect(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef typename Vectors<T>::Ty Ty;
  typedef typename Vectors<TI1>::Ty TyI1;
  volatile unsigned Values[] = {
      0x0,        0x1,        0x7ffffffe, 0x7fffffff, 0x80000000, 0x80000001,
      0xfffffffe, 0xffffffff, 0x7e,       0x7f,       0x80,       0x81,
      0xfe,       0xff,       0x100,      0x101,      0x7ffe,     0x7fff,
      0x8000,     0x8001,     0xfffe,     0xffff,     0x10000,    0x10001};
  static const size_t NumValues = sizeof(Values) / sizeof(*Values);
  static const size_t NumElements = Vectors<T>::NumElements;
  PRNG Index;
  for (size_t i = 0; i < MaxTestsPerFunc; ++i) {
    TyI1 Cond;
    Ty Value1, Value2;
    for (size_t j = 0; j < NumElements; ++j) {
      setElement(Cond, j, Index() % 2);
      setElement(Value1, j, Values[Index() % NumValues]);
      setElement(Value2, j, Values[Index() % NumValues]);
    }
    Ty ResultLlc = select(Cond, Value1, Value2);
    Ty ResultSz = Subzero_::select(Cond, Value1, Value2);
    ++TotalTests;
    if (!memcmp(&ResultLlc, &ResultSz, sizeof(ResultLlc))) {
      ++Passes;
    } else {
      ++Failures;
      std::cout << "select<" << Vectors<T>::TypeName << ">(Cond=";
      std::cout << vectAsString<TI1>(Cond)
                << ", Value1=" << vectAsString<T>(Value1)
                << ", Value2=" << vectAsString<T>(Value2) << ")\n";
      std::cout << "llc=" << vectAsString<T>(ResultLlc) << "\n";
      std::cout << "sz =" << vectAsString<T>(ResultSz) << "\n";
    }
  }
}

template <>
void testSelect<v4f32, v4i1>(size_t &TotalTests, size_t &Passes,
                             size_t &Failures) {
  static const float NegInf = -1.0 / 0.0;
  static const float PosInf = 1.0 / 0.0;
  static const float Nan = 0.0 / 0.0;
  static const float NegNan = -0.0 / 0.0;
  volatile float Values[] = FP_VALUE_ARRAY(NegInf, PosInf, NegNan, Nan);
  static const size_t NumValues = sizeof(Values) / sizeof(*Values);
  static const size_t NumElements = 4;
  PRNG Index;
  for (size_t i = 0; i < MaxTestsPerFunc; ++i) {
    v4si32 Cond;
    v4f32 Value1, Value2;
    for (size_t j = 0; j < NumElements; ++j) {
      setElement(Cond, j, Index() % 2);
      setElement(Value1, j, Values[Index() % NumValues]);
      setElement(Value2, j, Values[Index() % NumValues]);
    }
    v4f32 ResultLlc = select(Cond, Value1, Value2);
    v4f32 ResultSz = Subzero_::select(Cond, Value1, Value2);
    ++TotalTests;
    if (!memcmp(&ResultLlc, &ResultSz, sizeof(ResultLlc))) {
      ++Passes;
    } else {
      ++Failures;
      std::cout << "select<v4f32>(Cond=";
      std::cout << vectAsString<v4i1>(Cond)
                << ", Value1=" << vectAsString<v4f32>(Value1)
                << ", Value2=" << vectAsString<v4f32>(Value2) << ")\n";
      std::cout << "llc=" << vectAsString<v4f32>(ResultLlc) << "\n";
      std::cout << "sz =" << vectAsString<v4f32>(ResultSz) << "\n";
    }
  }
}

template <typename T>
void testSelectI1(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef typename Vectors<T>::Ty Ty;
  static const size_t NumElements = Vectors<T>::NumElements;
  PRNG Index;
  for (size_t i = 0; i < MaxTestsPerFunc; ++i) {
    Ty Cond;
    Ty Value1, Value2;
    for (size_t j = 0; j < NumElements; ++j) {
      setElement(Cond, j, Index() % 2);
      setElement(Value1, j, Index() % 2);
      setElement(Value2, j, Index() % 2);
    }
    Ty ResultLlc = select_i1(Cond, Value1, Value2);
    Ty ResultSz = Subzero_::select_i1(Cond, Value1, Value2);
    ++TotalTests;
    if (!memcmp(&ResultLlc, &ResultSz, sizeof(ResultLlc))) {
      ++Passes;
    } else {
      ++Failures;
      std::cout << "select<" << Vectors<T>::TypeName << ">(Cond=";
      std::cout << vectAsString<T>(Cond)
                << ", Value1=" << vectAsString<T>(Value1)
                << ", Value2=" << vectAsString<T>(Value2) << ")\n";
      std::cout << "llc=" << vectAsString<T>(ResultLlc) << "\n";
      std::cout << "sz =" << vectAsString<T>(ResultSz) << "\n";
    }
  }
}

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;

  testSelect<v4f32, v4i1>(TotalTests, Passes, Failures);
  testSelect<v4si32, v4i1>(TotalTests, Passes, Failures);
  testSelect<v4ui32, v4i1>(TotalTests, Passes, Failures);
  testSelect<v8si16, v8i1>(TotalTests, Passes, Failures);
  testSelect<v8ui16, v8i1>(TotalTests, Passes, Failures);
  testSelect<v16si8, v16i1>(TotalTests, Passes, Failures);
  testSelect<v16ui8, v16i1>(TotalTests, Passes, Failures);
  testSelectI1<v4i1>(TotalTests, Passes, Failures);
  testSelectI1<v8i1>(TotalTests, Passes, Failures);
  testSelectI1<v16i1>(TotalTests, Passes, Failures);

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";

  return Failures;
}
