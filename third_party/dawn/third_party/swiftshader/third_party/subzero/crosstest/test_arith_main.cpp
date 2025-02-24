//===- subzero/crosstest/test_arith_main.cpp - Driver for tests -----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for crosstesting arithmetic operations
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_arith.cpp --test=test_arith_frem.ll \
   --test=test_arith_sqrt.ll --driver=test_arith_main.cpp \
   --prefix=Subzero_ --output=test_arith */

#include <stdint.h>

#include <cfloat>
#include <climits> // CHAR_BIT
#include <cmath>   // fmodf
#include <cstring> // memcmp
#include <iostream>
#include <limits>

// Include test_arith.h twice - once normally, and once within the
// Subzero_ namespace, corresponding to the llc and Subzero translated
// object files, respectively.
#include "test_arith.h"

namespace Subzero_ {
#include "test_arith.h"
}

#include "insertelement.h"
#include "xdefs.h"

template <class T> bool inputsMayTriggerException(T Value1, T Value2) {
  // Avoid HW divide-by-zero exception.
  if (Value2 == 0)
    return true;
  // Avoid HW overflow exception (on x86-32).  TODO: adjust
  // for other architecture.
  if (Value1 == std::numeric_limits<T>::min() && Value2 == -1)
    return true;
  return false;
}

template <typename TypeUnsigned, typename TypeSigned>
void testsInt(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef TypeUnsigned (*FuncTypeUnsigned)(TypeUnsigned, TypeUnsigned);
  typedef TypeSigned (*FuncTypeSigned)(TypeSigned, TypeSigned);
  volatile unsigned Values[] = INT_VALUE_ARRAY;
  const static size_t NumValues = sizeof(Values) / sizeof(*Values);
  static struct {
    // For functions that operate on unsigned values, the
    // FuncLlcSigned and FuncSzSigned fields are NULL.  For functions
    // that operate on signed values, the FuncLlcUnsigned and
    // FuncSzUnsigned fields are NULL.
    const char *Name;
    FuncTypeUnsigned FuncLlcUnsigned;
    FuncTypeUnsigned FuncSzUnsigned;
    FuncTypeSigned FuncLlcSigned;
    FuncTypeSigned FuncSzSigned;
    bool ExcludeDivExceptions; // for divide related tests
  } Funcs[] = {
#define X(inst, op, isdiv, isshift)                                            \
  {STR(inst), test##inst, Subzero_::test##inst, NULL, NULL, isdiv},
      UINTOP_TABLE
#undef X
#define X(inst, op, isdiv, isshift)                                            \
  {STR(inst), NULL, NULL, test##inst, Subzero_::test##inst, isdiv},
          SINTOP_TABLE
#undef X
#define X(mult_by)                                                             \
  {"Mult-By-" STR(mult_by),                                                    \
   testMultiplyBy##mult_by,                                                    \
   Subzero_::testMultiplyBy##mult_by,                                          \
   NULL,                                                                       \
   NULL,                                                                       \
   false},                                                                     \
      {"Mult-By-Neg-" STR(mult_by),                                            \
       testMultiplyByNeg##mult_by,                                             \
       Subzero_::testMultiplyByNeg##mult_by,                                   \
       NULL,                                                                   \
       NULL,                                                                   \
       false},
              MULIMM_TABLE};
#undef X
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);

  if (sizeof(TypeUnsigned) <= sizeof(uint32_t)) {
    // This is the "normal" version of the loop nest, for 32-bit or
    // narrower types.
    for (size_t f = 0; f < NumFuncs; ++f) {
      for (size_t i = 0; i < NumValues; ++i) {
        for (size_t j = 0; j < NumValues; ++j) {
          TypeUnsigned Value1 = Values[i];
          TypeUnsigned Value2 = Values[j];
          // Avoid HW divide-by-zero exception.
          if (Funcs[f].ExcludeDivExceptions &&
              inputsMayTriggerException<TypeSigned>(Value1, Value2))
            continue;
          ++TotalTests;
          TypeUnsigned ResultSz, ResultLlc;
          if (Funcs[f].FuncSzUnsigned) {
            ResultSz = Funcs[f].FuncSzUnsigned(Value1, Value2);
            ResultLlc = Funcs[f].FuncLlcUnsigned(Value1, Value2);
          } else {
            ResultSz = Funcs[f].FuncSzSigned(Value1, Value2);
            ResultLlc = Funcs[f].FuncLlcSigned(Value1, Value2);
          }
          if (ResultSz == ResultLlc) {
            ++Passes;
          } else {
            ++Failures;
            std::cout << "test" << Funcs[f].Name
                      << (CHAR_BIT * sizeof(TypeUnsigned)) << "(" << Value1
                      << ", " << Value2 << "): sz=" << (unsigned)ResultSz
                      << " llc=" << (unsigned)ResultLlc << "\n";
          }
        }
      }
    }
  } else {
    // This is the 64-bit version.  Test values are synthesized from
    // the 32-bit values in Values[].
    for (size_t f = 0; f < NumFuncs; ++f) {
      for (size_t iLo = 0; iLo < NumValues; ++iLo) {
        for (size_t iHi = 0; iHi < NumValues; ++iHi) {
          for (size_t jLo = 0; jLo < NumValues; ++jLo) {
            for (size_t jHi = 0; jHi < NumValues; ++jHi) {
              TypeUnsigned Value1 =
                  (((TypeUnsigned)Values[iHi]) << 32) + Values[iLo];
              TypeUnsigned Value2 =
                  (((TypeUnsigned)Values[jHi]) << 32) + Values[jLo];
              if (Funcs[f].ExcludeDivExceptions &&
                  inputsMayTriggerException<TypeSigned>(Value1, Value2))
                continue;
              ++TotalTests;
              TypeUnsigned ResultSz, ResultLlc;
              if (Funcs[f].FuncSzUnsigned) {
                ResultSz = Funcs[f].FuncSzUnsigned(Value1, Value2);
                ResultLlc = Funcs[f].FuncLlcUnsigned(Value1, Value2);
              } else {
                ResultSz = Funcs[f].FuncSzSigned(Value1, Value2);
                ResultLlc = Funcs[f].FuncLlcSigned(Value1, Value2);
              }
              if (ResultSz == ResultLlc) {
                ++Passes;
              } else {
                ++Failures;
                std::cout << "test" << Funcs[f].Name
                          << (CHAR_BIT * sizeof(TypeUnsigned)) << "(" << Value1
                          << ", " << Value2 << "): sz=" << (uint64)ResultSz
                          << " llc=" << (uint64)ResultLlc << "\n";
              }
            }
          }
        }
      }
    }
  }
}

const static size_t MaxTestsPerFunc = 100000;

template <typename TypeUnsignedLabel, typename TypeSignedLabel>
void testsVecInt(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef typename Vectors<TypeUnsignedLabel>::Ty TypeUnsigned;
  typedef typename Vectors<TypeSignedLabel>::Ty TypeSigned;
  typedef typename Vectors<TypeUnsignedLabel>::ElementTy ElementTypeUnsigned;
  typedef typename Vectors<TypeSignedLabel>::ElementTy ElementTypeSigned;

  typedef TypeUnsigned (*FuncTypeUnsigned)(TypeUnsigned, TypeUnsigned);
  typedef TypeSigned (*FuncTypeSigned)(TypeSigned, TypeSigned);
  volatile unsigned Values[] = INT_VALUE_ARRAY;
  const static size_t NumValues = sizeof(Values) / sizeof(*Values);
  static struct {
    // For functions that operate on unsigned values, the
    // FuncLlcSigned and FuncSzSigned fields are NULL.  For functions
    // that operate on signed values, the FuncLlcUnsigned and
    // FuncSzUnsigned fields are NULL.
    const char *Name;
    FuncTypeUnsigned FuncLlcUnsigned;
    FuncTypeUnsigned FuncSzUnsigned;
    FuncTypeSigned FuncLlcSigned;
    FuncTypeSigned FuncSzSigned;
    bool ExcludeDivExceptions; // for divide related tests
    bool MaskShiftOperations;  // for shift related tests
  } Funcs[] = {
#define X(inst, op, isdiv, isshift)                                            \
  {STR(inst), test##inst, Subzero_::test##inst, NULL, NULL, isdiv, isshift},
      UINTOP_TABLE
#undef X
#define X(inst, op, isdiv, isshift)                                            \
  {STR(inst), NULL, NULL, test##inst, Subzero_::test##inst, isdiv, isshift},
          SINTOP_TABLE
#undef X
  };
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);
  const static size_t NumElementsInType = Vectors<TypeUnsigned>::NumElements;
  for (size_t f = 0; f < NumFuncs; ++f) {
    PRNG Index;
    for (size_t i = 0; i < MaxTestsPerFunc; ++i) {
      // Initialize the test vectors.
      TypeUnsigned Value1, Value2;
      for (size_t j = 0; j < NumElementsInType; ++j) {
        ElementTypeUnsigned Element1 = Values[Index() % NumValues];
        ElementTypeUnsigned Element2 = Values[Index() % NumValues];
        if (Funcs[f].ExcludeDivExceptions &&
            inputsMayTriggerException<ElementTypeSigned>(Element1, Element2))
          continue;
        if (Funcs[f].MaskShiftOperations)
          Element2 &= CHAR_BIT * sizeof(ElementTypeUnsigned) - 1;
        setElement(Value1, j, Element1);
        setElement(Value2, j, Element2);
      }
      // Perform the test.
      TypeUnsigned ResultSz, ResultLlc;
      ++TotalTests;
      if (Funcs[f].FuncSzUnsigned) {
        ResultSz = Funcs[f].FuncSzUnsigned(Value1, Value2);
        ResultLlc = Funcs[f].FuncLlcUnsigned(Value1, Value2);
      } else {
        ResultSz = Funcs[f].FuncSzSigned(Value1, Value2);
        ResultLlc = Funcs[f].FuncLlcSigned(Value1, Value2);
      }
      if (!memcmp(&ResultSz, &ResultLlc, sizeof(ResultSz))) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "test" << Funcs[f].Name << "v" << NumElementsInType << "i"
                  << (CHAR_BIT * sizeof(ElementTypeUnsigned)) << "("
                  << vectAsString<TypeUnsignedLabel>(Value1) << ","
                  << vectAsString<TypeUnsignedLabel>(Value2)
                  << "): sz=" << vectAsString<TypeUnsignedLabel>(ResultSz)
                  << " llc=" << vectAsString<TypeUnsignedLabel>(ResultLlc)
                  << "\n";
      }
    }
  }
}

template <typename Type>
void testsFp(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  static const Type NegInf = -1.0 / 0.0;
  static const Type PosInf = 1.0 / 0.0;
  static const Type Nan = 0.0 / 0.0;
  static const Type NegNan = -0.0 / 0.0;
  volatile Type Values[] = FP_VALUE_ARRAY(NegInf, PosInf, NegNan, Nan);
  const static size_t NumValues = sizeof(Values) / sizeof(*Values);
  typedef Type (*FuncType)(Type, Type);
  static struct {
    const char *Name;
    FuncType FuncLlc;
    FuncType FuncSz;
  } Funcs[] = {
#define X(inst, op, func)                                                      \
  {STR(inst), (FuncType)test##inst, (FuncType)Subzero_::test##inst},
      FPOP_TABLE
#undef X
  };
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);

  for (size_t f = 0; f < NumFuncs; ++f) {
    for (size_t i = 0; i < NumValues; ++i) {
      for (size_t j = 0; j < NumValues; ++j) {
        Type Value1 = Values[i];
        Type Value2 = Values[j];
        ++TotalTests;
        Type ResultSz = Funcs[f].FuncSz(Value1, Value2);
        Type ResultLlc = Funcs[f].FuncLlc(Value1, Value2);
        // Compare results using memcmp() in case they are both NaN.
        if (!memcmp(&ResultSz, &ResultLlc, sizeof(Type))) {
          ++Passes;
        } else {
          ++Failures;
          std::cout << std::fixed << "test" << Funcs[f].Name
                    << (CHAR_BIT * sizeof(Type)) << "(" << Value1 << ", "
                    << Value2 << "): sz=" << ResultSz << " llc=" << ResultLlc
                    << "\n";
        }
      }
    }
  }
  for (size_t i = 0; i < NumValues; ++i) {
    Type Value = Values[i];
    ++TotalTests;
    Type ResultSz = Subzero_::mySqrt(Value);
    Type ResultLlc = mySqrt(Value);
    // Compare results using memcmp() in case they are both NaN.
    if (!memcmp(&ResultSz, &ResultLlc, sizeof(Type))) {
      ++Passes;
    } else {
      ++Failures;
      std::cout << std::fixed << "test_sqrt" << (CHAR_BIT * sizeof(Type)) << "("
                << Value << "): sz=" << ResultSz << " llc=" << ResultLlc
                << "\n";
    }
    ++TotalTests;
    ResultSz = Subzero_::myFabs(Value);
    ResultLlc = myFabs(Value);
    // Compare results using memcmp() in case they are both NaN.
    if (!memcmp(&ResultSz, &ResultLlc, sizeof(Type))) {
      ++Passes;
    } else {
      ++Failures;
      std::cout << std::fixed << "test_fabs" << (CHAR_BIT * sizeof(Type)) << "("
                << Value << "): sz=" << ResultSz << " llc=" << ResultLlc
                << "\n";
    }
  }
}

void testsVecFp(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  static const float NegInf = -1.0 / 0.0;
  static const float PosInf = 1.0 / 0.0;
  static const float Nan = 0.0 / 0.0;
  static const float NegNan = -0.0 / 0.0;
  volatile float Values[] = FP_VALUE_ARRAY(NegInf, PosInf, NegNan, Nan);
  const static size_t NumValues = sizeof(Values) / sizeof(*Values);
  typedef v4f32 (*FuncType)(v4f32, v4f32);
  static struct {
    const char *Name;
    FuncType FuncLlc;
    FuncType FuncSz;
  } Funcs[] = {
#define X(inst, op, func)                                                      \
  {STR(inst), (FuncType)test##inst, (FuncType)Subzero_::test##inst},
      FPOP_TABLE
#undef X
  };
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);
  const static size_t NumElementsInType = 4;
  for (size_t f = 0; f < NumFuncs; ++f) {
    PRNG Index;
    for (size_t i = 0; i < MaxTestsPerFunc; ++i) {
      // Initialize the test vectors.
      v4f32 Value1, Value2;
      for (size_t j = 0; j < NumElementsInType; ++j) {
        setElement(Value1, j, Values[Index() % NumValues]);
        setElement(Value2, j, Values[Index() % NumValues]);
      }
      // Perform the test.
      v4f32 ResultSz = Funcs[f].FuncSz(Value1, Value2);
      v4f32 ResultLlc = Funcs[f].FuncLlc(Value1, Value2);
      ++TotalTests;
      if (!memcmp(&ResultSz, &ResultLlc, sizeof(ResultSz))) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "test" << Funcs[f].Name << "v4f32"
                  << "(" << vectAsString<v4f32>(Value1) << ","
                  << vectAsString<v4f32>(Value2)
                  << "): sz=" << vectAsString<v4f32>(ResultSz) << " llc"
                  << vectAsString<v4f32>(ResultLlc) << "\n";
      }
      // Special case for unary fabs operation.  Use Value1, ignore Value2.
      ResultSz = Subzero_::myFabs(Value1);
      ResultLlc = myFabs(Value1);
      ++TotalTests;
      if (!memcmp(&ResultSz, &ResultLlc, sizeof(ResultSz))) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "test_fabs_v4f32"
                  << "(" << vectAsString<v4f32>(Value1)
                  << "): sz=" << vectAsString<v4f32>(ResultSz) << " llc"
                  << vectAsString<v4f32>(ResultLlc) << "\n";
      }
    }
  }
}

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;

  testsInt<bool, bool>(TotalTests, Passes, Failures);
  testsInt<uint8_t, myint8_t>(TotalTests, Passes, Failures);
  testsInt<uint16_t, int16_t>(TotalTests, Passes, Failures);
  testsInt<uint32_t, int32_t>(TotalTests, Passes, Failures);
  testsInt<uint64, int64>(TotalTests, Passes, Failures);
  testsVecInt<v4ui32, v4si32>(TotalTests, Passes, Failures);
  testsVecInt<v8ui16, v8si16>(TotalTests, Passes, Failures);
  testsVecInt<v16ui8, v16si8>(TotalTests, Passes, Failures);
  testsFp<float>(TotalTests, Passes, Failures);
  testsFp<double>(TotalTests, Passes, Failures);
  testsVecFp(TotalTests, Passes, Failures);

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";
  return Failures;
}
