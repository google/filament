//===- subzero/crosstest/test_strengthreduce_main.cpp - Driver for tests --===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for crosstesting arithmetic strength-reducing optimizations.
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_strengthreduce.cpp \
   --driver=test_strengthreduce_main.cpp \
   --prefix=Subzero_ --clang-opt=0 --output=test_strengthreduce */

#include <iostream>

// Include test_strengthreduce.h twice - once normally, and once
// within the Subzero_ namespace, corresponding to the llc and Subzero
// translated object files, respectively.
#include "test_strengthreduce.h"
namespace Subzero_ {
#include "test_strengthreduce.h"
}

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;
  static int32_t Values[] = {-100, -50, 0, 1, 8, 123, 0x33333333, 0x77777777};
  for (size_t i = 0; i < sizeof(Values) / sizeof(*Values); ++i) {
    int32_t SVal = Values[i];
    int32_t ResultLlcS, ResultSzS;
    uint32_t UVal = (uint32_t)Values[i];
    int32_t ResultLlcU, ResultSzU;

#define X(constant, suffix)                                                    \
  ResultLlcS = multiplyByConst##suffix(UVal);                                  \
  ResultSzS = Subzero_::multiplyByConst##suffix(UVal);                         \
  if (ResultLlcS == ResultSzS) {                                               \
    ++Passes;                                                                  \
  } else {                                                                     \
    ++Failures;                                                                \
    std::cout << "multiplyByConstS" STR(suffix) "(" << SVal                    \
              << "): sz=" << ResultSzS << " llc=" << ResultLlcS << "\n";       \
  }                                                                            \
  ResultLlcU = multiplyByConst##suffix(UVal);                                  \
  ResultSzU = Subzero_::multiplyByConst##suffix(UVal);                         \
  if (ResultLlcU == ResultSzU) {                                               \
    ++Passes;                                                                  \
  } else {                                                                     \
    ++Failures;                                                                \
    std::cout << "multiplyByConstU" STR(suffix) "(" << UVal                    \
              << "): sz=" << ResultSzU << " llc=" << ResultLlcU << "\n";       \
  }
    CONST_TABLE
#undef X
  }

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";
  return Failures;
}
