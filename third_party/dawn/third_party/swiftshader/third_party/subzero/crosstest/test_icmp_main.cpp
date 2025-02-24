//===- subzero/crosstest/test_icmp_main.cpp - Driver for tests. -----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for cross testing the icmp bitcode instruction
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_icmp.cpp --test=test_icmp_i1vec.ll \
   --driver=test_icmp_main.cpp --prefix=Subzero_ --output=test_icmp */

#include <climits> // CHAR_BIT
#include <cstring> // memcmp, memset
#include <iostream>
#include <stdint.h>

// Include test_icmp.h twice - once normally, and once within the
// Subzero_ namespace, corresponding to the llc and Subzero translated
// object files, respectively.
#include "test_icmp.h"

namespace Subzero_ {
#include "test_icmp.h"
}

#include "insertelement.h"
#include "xdefs.h"

volatile unsigned Values[] = {
    0x0,        0x1,        0x7ffffffe, 0x7fffffff, 0x80000000, 0x80000001,
    0xfffffffe, 0xffffffff, 0x7e,       0x7f,       0x80,       0x81,
    0xfe,       0xff,       0x100,      0x101,      0x7ffe,     0x7fff,
    0x8000,     0x8001,     0xfffe,     0xffff,     0x10000,    0x10001,
};
const static size_t NumValues = sizeof(Values) / sizeof(*Values);

template <typename TypeUnsigned, typename TypeSigned>
void testsInt(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef bool (*FuncTypeUnsigned)(TypeUnsigned, TypeUnsigned);
  typedef bool (*FuncTypeSigned)(TypeSigned, TypeSigned);
  static struct {
    const char *Name;
    FuncTypeUnsigned FuncLlc;
    FuncTypeUnsigned FuncSz;
  } Funcs[] = {
#define X(cmp, op)                                                             \
  {STR(cmp), (FuncTypeUnsigned)icmp##cmp,                                      \
   (FuncTypeUnsigned)Subzero_::icmp##cmp},
      ICMP_U_TABLE
#undef X
#define X(cmp, op)                                                             \
  {STR(cmp), (FuncTypeUnsigned)(FuncTypeSigned)icmp##cmp,                      \
   (FuncTypeUnsigned)(FuncTypeSigned)Subzero_::icmp##cmp},
          ICMP_S_TABLE
#undef X
  };
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);

  if (sizeof(TypeUnsigned) <= sizeof(uint32_t)) {
    // This is the "normal" version of the loop nest, for 32-bit or
    // narrower types.
    for (size_t f = 0; f < NumFuncs; ++f) {
      for (size_t i = 0; i < NumValues; ++i) {
        for (size_t j = 0; j < NumValues; ++j) {
          TypeUnsigned Value1 = Values[i];
          TypeUnsigned Value2 = Values[j];
          ++TotalTests;
          bool ResultSz = Funcs[f].FuncSz(Value1, Value2);
          bool ResultLlc = Funcs[f].FuncLlc(Value1, Value2);
          if (ResultSz == ResultLlc) {
            ++Passes;
          } else {
            ++Failures;
            std::cout << "icmp" << Funcs[f].Name
                      << (CHAR_BIT * sizeof(TypeUnsigned)) << "(" << Value1
                      << ", " << Value2 << "): sz=" << ResultSz
                      << " llc=" << ResultLlc << "\n";
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
              ++TotalTests;
              bool ResultSz = Funcs[f].FuncSz(Value1, Value2);
              bool ResultLlc = Funcs[f].FuncLlc(Value1, Value2);
              if (ResultSz == ResultLlc) {
                ++Passes;
              } else {
                ++Failures;
                std::cout << "icmp" << Funcs[f].Name
                          << (CHAR_BIT * sizeof(TypeUnsigned)) << "(" << Value1
                          << ", " << Value2 << "): sz=" << ResultSz
                          << " llc=" << ResultLlc << "\n";
              }
            }
          }
        }
      }
    }
  }
}

template <typename TypeUnsigned, typename TypeSigned>
void testsIntWithZero(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef bool (*FuncTypeUnsigned)(TypeUnsigned);
  typedef bool (*FuncTypeSigned)(TypeSigned);
  static struct {
    const char *Name;
    FuncTypeUnsigned FuncLlc;
    FuncTypeUnsigned FuncSz;
  } Funcs[] = {
#define X(cmp, op)                                                             \
  {STR(cmp), (FuncTypeUnsigned)icmp_zero##cmp,                                 \
   (FuncTypeUnsigned)Subzero_::icmp_zero##cmp},
      ICMP_U_TABLE
#undef X
#define X(cmp, op)                                                             \
  {STR(cmp), (FuncTypeUnsigned)(FuncTypeSigned)icmp_zero##cmp,                 \
   (FuncTypeUnsigned)(FuncTypeSigned)Subzero_::icmp_zero##cmp},
          ICMP_S_TABLE
#undef X
  };
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);

  if (sizeof(TypeUnsigned) <= sizeof(uint32_t)) {
    // This is the "normal" version of the loop nest, for 32-bit or
    // narrower types.
    for (size_t f = 0; f < NumFuncs; ++f) {
      for (size_t i = 0; i < NumValues; ++i) {
        TypeUnsigned Value = Values[i];
        ++TotalTests;
        bool ResultSz = Funcs[f].FuncSz(Value);
        bool ResultLlc = Funcs[f].FuncLlc(Value);
        if (ResultSz == ResultLlc) {
          ++Passes;
        } else {
          ++Failures;
          std::cout << "icmp" << Funcs[f].Name
                    << (CHAR_BIT * sizeof(TypeUnsigned)) << "(" << Value
                    << "): sz=" << ResultSz << " llc=" << ResultLlc << "\n";
        }
      }
    }
  } else {
    // This is the 64-bit version.  Test values are synthesized from
    // the 32-bit values in Values[].
    for (size_t f = 0; f < NumFuncs; ++f) {
      for (size_t iLo = 0; iLo < NumValues; ++iLo) {
        for (size_t iHi = 0; iHi < NumValues; ++iHi) {
          TypeUnsigned Value =
              (((TypeUnsigned)Values[iHi]) << 32) + Values[iLo];
          ++TotalTests;
          bool ResultSz = Funcs[f].FuncSz(Value);
          bool ResultLlc = Funcs[f].FuncLlc(Value);
          if (ResultSz == ResultLlc) {
            ++Passes;
          } else {
            ++Failures;
            std::cout << "icmp" << Funcs[f].Name
                      << (CHAR_BIT * sizeof(TypeUnsigned)) << "(" << Value
                      << "): sz=" << ResultSz << " llc=" << ResultLlc << "\n";
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
  static struct {
    const char *Name;
    FuncTypeUnsigned FuncLlc;
    FuncTypeUnsigned FuncSz;
  } Funcs[] = {
#define X(cmp, op)                                                             \
  {STR(cmp), (FuncTypeUnsigned)icmp##cmp,                                      \
   (FuncTypeUnsigned)Subzero_::icmp##cmp},
      ICMP_U_TABLE
#undef X
#define X(cmp, op)                                                             \
  {STR(cmp), (FuncTypeUnsigned)(FuncTypeSigned)icmp##cmp,                      \
   (FuncTypeUnsigned)(FuncTypeSigned)Subzero_::icmp##cmp},
          ICMP_S_TABLE
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
        setElement(Value1, j, Values[Index() % NumValues]);
        setElement(Value2, j, Values[Index() % NumValues]);
      }
      // Perform the test.
      TypeUnsigned ResultSz = Funcs[f].FuncSz(Value1, Value2);
      TypeUnsigned ResultLlc = Funcs[f].FuncLlc(Value1, Value2);
      ++TotalTests;
      if (!memcmp(&ResultSz, &ResultLlc, sizeof(ResultSz))) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "test" << Funcs[f].Name
                  << Vectors<TypeUnsignedLabel>::TypeName << "("
                  << vectAsString<TypeUnsignedLabel>(Value1) << ","
                  << vectAsString<TypeUnsignedLabel>(Value2)
                  << "): sz=" << vectAsString<TypeUnsignedLabel>(ResultSz)
                  << " llc=" << vectAsString<TypeUnsignedLabel>(ResultLlc)
                  << "\n";
      }
    }
  }
}

// Return true on wraparound
template <typename T>
bool __attribute__((noinline))
incrementI1Vector(typename Vectors<T>::Ty &Vect) {
  size_t Pos = 0;
  const static size_t NumElements = Vectors<T>::NumElements;
  for (Pos = 0; Pos < NumElements; ++Pos) {
    if (Vect[Pos] == 0) {
      Vect[Pos] = 1;
      break;
    }
    Vect[Pos] = 0;
  }
  return (Pos == NumElements);
}

template <typename T>
void testsVecI1(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef typename Vectors<T>::Ty Ty;
  typedef Ty (*FuncType)(Ty, Ty);
  static struct {
    const char *Name;
    FuncType FuncLlc;
    FuncType FuncSz;
  } Funcs[] = {
#define X(cmp, op)                                                             \
  {STR(cmp), (FuncType)icmpi1##cmp, (FuncType)Subzero_::icmpi1##cmp},
      ICMP_U_TABLE ICMP_S_TABLE};
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);
  const static size_t NumElements = Vectors<T>::NumElements;
  const static size_t MAX_NUMBER_OF_ELEMENTS_FOR_EXHAUSTIVE_TESTING = 8;

  // Check if the type is small enough to try all possible input pairs.
  if (NumElements <= MAX_NUMBER_OF_ELEMENTS_FOR_EXHAUSTIVE_TESTING) {
    for (size_t f = 0; f < NumFuncs; ++f) {
      Ty Value1, Value2;
      memset(&Value1, 0, sizeof(Value1));
      for (bool IsValue1Done = false; !IsValue1Done;
           IsValue1Done = incrementI1Vector<T>(Value1)) {
        memset(&Value2, 0, sizeof(Value2));
        for (bool IsValue2Done = false; !IsValue2Done;
             IsValue2Done = incrementI1Vector<T>(Value2)) {
          Ty ResultSz = Funcs[f].FuncSz(Value1, Value2);
          Ty ResultLlc = Funcs[f].FuncLlc(Value1, Value2);
          ++TotalTests;
          if (!memcmp(&ResultSz, &ResultLlc, sizeof(ResultSz))) {
            ++Passes;
          } else {
            ++Failures;
            std::cout << "test" << Funcs[f].Name << Vectors<T>::TypeName << "("
                      << vectAsString<T>(Value1) << ","
                      << vectAsString<T>(Value2)
                      << "): sz=" << vectAsString<T>(ResultSz)
                      << " llc=" << vectAsString<T>(ResultLlc) << "\n";
          }
        }
      }
    }
  } else {
    for (size_t f = 0; f < NumFuncs; ++f) {
      PRNG Index;
      for (size_t i = 0; i < MaxTestsPerFunc; ++i) {
        Ty Value1, Value2;
        // Initialize the test vectors.
        for (size_t j = 0; j < NumElements; ++j) {
          setElement(Value1, j, Index() % 2);
          setElement(Value2, j, Index() % 2);
        }
        // Perform the test.
        Ty ResultSz = Funcs[f].FuncSz(Value1, Value2);
        Ty ResultLlc = Funcs[f].FuncLlc(Value1, Value2);
        ++TotalTests;
        if (!memcmp(&ResultSz, &ResultLlc, sizeof(ResultSz))) {
          ++Passes;
        } else {
          ++Failures;
          std::cout << "test" << Funcs[f].Name << Vectors<T>::TypeName << "("
                    << vectAsString<T>(Value1) << "," << vectAsString<T>(Value2)
                    << "): sz=" << vectAsString<T>(ResultSz)
                    << " llc=" << vectAsString<T>(ResultLlc) << "\n";
        }
      }
    }
  }
}

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;

  testsInt<uint8_t, myint8_t>(TotalTests, Passes, Failures);
  testsInt<uint16_t, int16_t>(TotalTests, Passes, Failures);
  testsInt<uint32_t, int32_t>(TotalTests, Passes, Failures);
  testsInt<uint64, int64>(TotalTests, Passes, Failures);
  testsIntWithZero<uint8_t, myint8_t>(TotalTests, Passes, Failures);
  testsIntWithZero<uint16_t, int16_t>(TotalTests, Passes, Failures);
  testsIntWithZero<uint32_t, int32_t>(TotalTests, Passes, Failures);
  testsIntWithZero<uint64, int64>(TotalTests, Passes, Failures);
  testsVecInt<v4ui32, v4si32>(TotalTests, Passes, Failures);
  testsVecInt<v8ui16, v8si16>(TotalTests, Passes, Failures);
  testsVecInt<v16ui8, v16si8>(TotalTests, Passes, Failures);
  testsVecI1<v4i1>(TotalTests, Passes, Failures);
  testsVecI1<v8i1>(TotalTests, Passes, Failures);
  testsVecI1<v16i1>(TotalTests, Passes, Failures);

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";
  return Failures;
}
