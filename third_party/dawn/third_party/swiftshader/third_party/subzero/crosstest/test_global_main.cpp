//===- subzero/crosstest/test_global_main.cpp - Driver for tests ----------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for crosstesting global variable access operations.
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_global.cpp \
   --driver=test_global_main.cpp --prefix=Subzero_ --output=test_global */

#include <cstdlib>
#include <iostream>
#include <stdint.h>

#include "test_global.h"
namespace Subzero_ {
#include "test_global.h"
}

int ExternName1 = 36363;
float ExternName2 = 357.05e-10;
char ExternName3[] = {'a', 'b', 'c'};
struct Data {
  int a;
  float b;
  double d;
};

struct Data SimpleData = {-111, 2.69, 55.19};

struct Data *ExternName4 = &SimpleData;

double ExternName5 = 3.44e26;

int main(int argc, char **argv) {
  // Prevent pnacl-opt from deleting "unused" globals.
  if (argc < 0) {
    std::cout << &ExternName1 << &ExternName2 << &ExternName3 << &SimpleData
              << &ExternName4 << ExternName5;
  }
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;

  const uint8_t *SzArray, *LlcArray;
  size_t SzArrayLen, LlcArrayLen;

  size_t NumArrays = getNumArrays();
  for (size_t i = 0; i < NumArrays; ++i) {
    LlcArrayLen = -1;
    SzArrayLen = -2;
    LlcArray = getArray(i, LlcArrayLen);
    SzArray = Subzero_::getArray(i, SzArrayLen);
    ++TotalTests;
    if (LlcArrayLen == SzArrayLen) {
      ++Passes;
    } else {
      std::cout << i << ":LlcArrayLen=" << LlcArrayLen
                << ", SzArrayLen=" << SzArrayLen << "\n";
      ++Failures;
    }

    for (size_t i = 0; i < LlcArrayLen; ++i) {
      ++TotalTests;
      if (LlcArray[i] == SzArray[i]) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << i << ":LlcArray[" << i << "] = " << (int)LlcArray[i]
                  << ", SzArray[" << i << "] = " << (int)SzArray[i] << "\n";
      }
    }
  }

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";
  return Failures;
}
