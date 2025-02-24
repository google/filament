//===- subzero/crosstest/test_cast_main.cpp - Driver for tests ------------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for crosstesting cast operations.
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_cast.cpp --test=test_cast_to_u1.ll \
   --test=test_cast_vectors.ll \
   --driver=test_cast_main.cpp --prefix=Subzero_ --output=test_cast */

#include <cfloat>
#include <cstring>
#include <iostream>
#include <stdint.h>

#include "test_arith.def"
#include "vectors.h"
#include "xdefs.h"

// Include test_cast.h twice - once normally, and once within the
// Subzero_ namespace, corresponding to the llc and Subzero translated
// object files, respectively.
#include "test_cast.h"
namespace Subzero_ {
#include "test_cast.h"
}

#define XSTR(s) STR(s)
#define STR(s) #s
#define COMPARE(Func, FromCName, ToCName, Input, FromString)                   \
  do {                                                                         \
    ToCName ResultSz, ResultLlc;                                               \
    ResultLlc = Func<FromCName, ToCName>(Input);                               \
    ResultSz = Subzero_::Func<FromCName, ToCName>(Input);                      \
    ++TotalTests;                                                              \
    if (!memcmp(&ResultLlc, &ResultSz, sizeof(ToCName))) {                     \
      ++Passes;                                                                \
    } else {                                                                   \
      ++Failures;                                                              \
      std::cout << std::fixed << XSTR(Func) << "<" << FromString               \
                << ", " XSTR(ToCName) ">(" << Input << "): ";                  \
      if (sizeof(ToCName) == 1)                                                \
        std::cout << "sz=" << (int)ResultSz << " llc=" << (int)ResultLlc;      \
      else                                                                     \
        std::cout << "sz=" << ResultSz << " llc=" << ResultLlc;                \
      std::cout << "\n";                                                       \
    }                                                                          \
  } while (0)

#define COMPARE_ARG(Func, FromCName, ToCName, Input, FromString)               \
  do {                                                                         \
    ToCName ResultSz, ResultLlc;                                               \
    ResultLlc = Func<FromCName, ToCName>(1, Input, 2);                         \
    ResultSz = Subzero_::Func<FromCName, ToCName>(1, Input, 2);                \
    ++TotalTests;                                                              \
    if (!memcmp(&ResultLlc, &ResultSz, sizeof(ToCName))) {                     \
      ++Passes;                                                                \
    } else {                                                                   \
      ++Failures;                                                              \
      std::cout << std::fixed << XSTR(Func) << "<" << FromString               \
                << ", " XSTR(ToCName) ">(" << Input << "): ";                  \
      if (sizeof(ToCName) == 1)                                                \
        std::cout << "sz=" << (int)ResultSz << " llc=" << (int)ResultLlc;      \
      else                                                                     \
        std::cout << "sz=" << ResultSz << " llc=" << ResultLlc;                \
      std::cout << "\n";                                                       \
    }                                                                          \
  } while (0)

#define COMPARE_VEC(Func, FromCName, ToCName, Input, FromString, ToString)     \
  do {                                                                         \
    ToCName ResultSz, ResultLlc;                                               \
    ResultLlc = Func<FromCName, ToCName>(Input);                               \
    ResultSz = Subzero_::Func<FromCName, ToCName>(Input);                      \
    ++TotalTests;                                                              \
    if (!memcmp(&ResultLlc, &ResultSz, sizeof(ToCName))) {                     \
      ++Passes;                                                                \
    } else {                                                                   \
      ++Failures;                                                              \
      std::cout << std::fixed << XSTR(Func) << "<" << FromString << ", "       \
                << ToString << ">(" << vectAsString<FromCName>(Input)          \
                << "): ";                                                      \
      std::cout << "sz=" << vectAsString<ToCName>(ResultSz)                    \
                << " llc=" << vectAsString<ToCName>(ResultLlc);                \
      std::cout << "\n";                                                       \
    }                                                                          \
  } while (0)

template <typename FromType>
void testValue(FromType Val, size_t &TotalTests, size_t &Passes,
               size_t &Failures, const char *FromTypeString) {
  COMPARE(cast, FromType, bool, Val, FromTypeString);
  COMPARE(cast, FromType, uint8_t, Val, FromTypeString);
  COMPARE(cast, FromType, myint8_t, Val, FromTypeString);
  COMPARE(cast, FromType, uint16_t, Val, FromTypeString);
  COMPARE(cast, FromType, int16_t, Val, FromTypeString);
  COMPARE(cast, FromType, uint32_t, Val, FromTypeString);
  COMPARE(cast, FromType, int32_t, Val, FromTypeString);
  COMPARE(cast, FromType, uint64, Val, FromTypeString);
  COMPARE(cast, FromType, int64, Val, FromTypeString);
  COMPARE(cast, FromType, float, Val, FromTypeString);
  COMPARE(cast, FromType, double, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, bool, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, uint8_t, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, myint8_t, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, uint16_t, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, int16_t, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, uint32_t, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, int32_t, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, uint64, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, int64, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, float, Val, FromTypeString);
  COMPARE_ARG(cast, FromType, double, Val, FromTypeString);
}

template <typename FromType, typename ToType>
void testVector(size_t &TotalTests, size_t &Passes, size_t &Failures,
                const char *FromTypeString, const char *ToTypeString) {
  const static size_t NumElementsInType = Vectors<FromType>::NumElements;
  PRNG Index;
  static const float NegInf = -1.0 / 0.0;
  static const float PosInf = 1.0 / 0.0;
  static const float Nan = 0.0 / 0.0;
  static const float NegNan = -0.0 / 0.0;
  volatile float Values[] = FP_VALUE_ARRAY(NegInf, PosInf, NegNan, Nan);
  static const size_t NumValues = sizeof(Values) / sizeof(*Values);
  const size_t MaxTestsPerFunc = 20000;
  for (size_t i = 0; i < MaxTestsPerFunc; ++i) {
    // Initialize the test vectors.
    FromType Value;
    for (size_t j = 0; j < NumElementsInType; ++j) {
      Value[j] = Values[Index() % NumValues];
    }
    COMPARE_VEC(cast, FromType, ToType, Value, FromTypeString, ToTypeString);
  }
}

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;

  volatile bool ValsUi1[] = {false, true};
  static const size_t NumValsUi1 = sizeof(ValsUi1) / sizeof(*ValsUi1);
  volatile uint8_t ValsUi8[] = {0, 1, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff};
  static const size_t NumValsUi8 = sizeof(ValsUi8) / sizeof(*ValsUi8);

  volatile myint8_t ValsSi8[] = {0, 1, 0x7e, 0x7f, 0x80, 0x81, 0xfe, 0xff};
  static const size_t NumValsSi8 = sizeof(ValsSi8) / sizeof(*ValsSi8);

  volatile uint16_t ValsUi16[] = {0,      1,      0x7e,   0x7f,   0x80,
                                  0x81,   0xfe,   0xff,   0x7ffe, 0x7fff,
                                  0x8000, 0x8001, 0xfffe, 0xffff};
  static const size_t NumValsUi16 = sizeof(ValsUi16) / sizeof(*ValsUi16);

  volatile int16_t ValsSi16[] = {0,      1,      0x7e,   0x7f,   0x80,
                                 0x81,   0xfe,   0xff,   0x7ffe, 0x7fff,
                                 0x8000, 0x8001, 0xfffe, 0xffff};
  static const size_t NumValsSi16 = sizeof(ValsSi16) / sizeof(*ValsSi16);

  volatile size_t ValsUi32[] = {0,          1,          0x7e,       0x7f,
                                0x80,       0x81,       0xfe,       0xff,
                                0x7ffe,     0x7fff,     0x8000,     0x8001,
                                0xfffe,     0xffff,     0x7ffffffe, 0x7fffffff,
                                0x80000000, 0x80000001, 0xfffffffe, 0xffffffff};
  static const size_t NumValsUi32 = sizeof(ValsUi32) / sizeof(*ValsUi32);

  volatile size_t ValsSi32[] = {0,          1,          0x7e,       0x7f,
                                0x80,       0x81,       0xfe,       0xff,
                                0x7ffe,     0x7fff,     0x8000,     0x8001,
                                0xfffe,     0xffff,     0x7ffffffe, 0x7fffffff,
                                0x80000000, 0x80000001, 0xfffffffe, 0xffffffff};
  static const size_t NumValsSi32 = sizeof(ValsSi32) / sizeof(*ValsSi32);

  volatile uint64 ValsUi64[] = {0,
                                1,
                                0x7e,
                                0x7f,
                                0x80,
                                0x81,
                                0xfe,
                                0xff,
                                0x7ffe,
                                0x7fff,
                                0x8000,
                                0x8001,
                                0xfffe,
                                0xffff,
                                0x7ffffffe,
                                0x7fffffff,
                                0x80000000,
                                0x80000001,
                                0xfffffffe,
                                0xffffffff,
                                0x100000000ull,
                                0x100000001ull,
                                0x7ffffffffffffffeull,
                                0x7fffffffffffffffull,
                                0x8000000000000000ull,
                                0x8000000000000001ull,
                                0xfffffffffffffffeull,
                                0xffffffffffffffffull};
  static const size_t NumValsUi64 = sizeof(ValsUi64) / sizeof(*ValsUi64);

  volatile int64 ValsSi64[] = {0,
                               1,
                               0x7e,
                               0x7f,
                               0x80,
                               0x81,
                               0xfe,
                               0xff,
                               0x7ffe,
                               0x7fff,
                               0x8000,
                               0x8001,
                               0xfffe,
                               0xffff,
                               0x7ffffffe,
                               0x7fffffff,
                               0x80000000,
                               0x80000001,
                               0xfffffffe,
                               0xffffffff,
                               0x100000000ll,
                               0x100000001ll,
                               0x7ffffffffffffffell,
                               0x7fffffffffffffffll,
                               0x8000000000000000ll,
                               0x8000000000000001ll,
                               0xfffffffffffffffell,
                               0xffffffffffffffffll};
  static const size_t NumValsSi64 = sizeof(ValsSi64) / sizeof(*ValsSi64);

  static const double NegInf = -1.0 / 0.0;
  static const double PosInf = 1.0 / 0.0;
  static const double Nan = 0.0 / 0.0;
  static const double NegNan = -0.0 / 0.0;
  volatile float ValsF32[] = FP_VALUE_ARRAY(NegInf, PosInf, NegNan, Nan);
  static const size_t NumValsF32 = sizeof(ValsF32) / sizeof(*ValsF32);

  volatile double ValsF64[] = FP_VALUE_ARRAY(NegInf, PosInf, NegNan, Nan);
  static const size_t NumValsF64 = sizeof(ValsF64) / sizeof(*ValsF64);

  for (size_t i = 0; i < NumValsUi1; ++i) {
    bool Val = ValsUi1[i];
    testValue<bool>(Val, TotalTests, Passes, Failures, "bool");
  }
  for (size_t i = 0; i < NumValsUi8; ++i) {
    uint8_t Val = ValsUi8[i];
    testValue<uint8_t>(Val, TotalTests, Passes, Failures, "uint8_t");
  }
  for (size_t i = 0; i < NumValsSi8; ++i) {
    myint8_t Val = ValsSi8[i];
    testValue<myint8_t>(Val, TotalTests, Passes, Failures, "int8_t");
  }
  for (size_t i = 0; i < NumValsUi16; ++i) {
    uint16_t Val = ValsUi16[i];
    testValue<uint16_t>(Val, TotalTests, Passes, Failures, "uint16_t");
  }
  for (size_t i = 0; i < NumValsSi16; ++i) {
    int16_t Val = ValsSi16[i];
    testValue<int16_t>(Val, TotalTests, Passes, Failures, "int16_t");
  }
  for (size_t i = 0; i < NumValsUi32; ++i) {
    uint32_t Val = ValsUi32[i];
    testValue<uint32_t>(Val, TotalTests, Passes, Failures, "uint32_t");
    COMPARE(castBits, uint32_t, float, Val, "uint32_t");
    COMPARE_ARG(castBits, uint32_t, float, Val, "uint32_t");
  }
  for (size_t i = 0; i < NumValsSi32; ++i) {
    int32_t Val = ValsSi32[i];
    testValue<int32_t>(Val, TotalTests, Passes, Failures, "int32_t");
  }
  for (size_t i = 0; i < NumValsUi64; ++i) {
    uint64 Val = ValsUi64[i];
    testValue<uint64>(Val, TotalTests, Passes, Failures, "uint64");
    COMPARE(castBits, uint64, double, Val, "uint64");
    COMPARE_ARG(castBits, uint64, double, Val, "uint64");
  }
  for (size_t i = 0; i < NumValsSi64; ++i) {
    int64 Val = ValsSi64[i];
    testValue<int64>(Val, TotalTests, Passes, Failures, "int64");
  }
  for (size_t i = 0; i < NumValsF32; ++i) {
    for (unsigned j = 0; j < 2; ++j) {
      float Val = ValsF32[i];
      if (j > 0)
        Val = -Val;
      testValue<float>(Val, TotalTests, Passes, Failures, "float");
      COMPARE(castBits, float, uint32_t, Val, "float");
      COMPARE_ARG(castBits, float, uint32_t, Val, "float");
    }
  }
  for (size_t i = 0; i < NumValsF64; ++i) {
    for (unsigned j = 0; j < 2; ++j) {
      double Val = ValsF64[i];
      if (j > 0)
        Val = -Val;
      testValue<double>(Val, TotalTests, Passes, Failures, "double");
      COMPARE(castBits, double, uint64, Val, "double");
      COMPARE_ARG(castBits, double, uint64, Val, "double");
    }
  }

  testVector<v4ui32, v4f32>(TotalTests, Passes, Failures, "v4ui32", "v4f32");
  testVector<v4si32, v4f32>(TotalTests, Passes, Failures, "v4si32", "v4f32");
  testVector<v4f32, v4si32>(TotalTests, Passes, Failures, "v4f32", "v4si32");
  testVector<v4f32, v4ui32>(TotalTests, Passes, Failures, "v4f32", "v4ui32");

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";
  return Failures;
}
