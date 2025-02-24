//===- subzero/crosstest/test_stacksave_main.c - Driver for tests ---------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for cross testing stacksave/stackrestore intrinsics.
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_stacksave.c --driver=test_stacksave_main.c \
   --prefix=Subzero_ --output=test_stacksave */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include "test_stacksave.h"
DECLARE_TESTS()
DECLARE_TESTS(Subzero_)

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;
  typedef uint32_t (*FuncType)(uint32_t, uint32_t, uint32_t);
  static struct {
    const char *Name;
    FuncType FuncLlc;
    FuncType FuncSz;
  } Funcs[] = {{"test_basic_vla", test_basic_vla, Subzero_test_basic_vla},
               {"test_vla_in_loop", test_vla_in_loop, Subzero_test_vla_in_loop},
               {"test_two_vlas_in_loops", test_two_vlas_in_loops,
                Subzero_test_two_vlas_in_loops}};
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);
  const uint32_t size_to_test = 128;
  for (size_t f = 0; f < NumFuncs; ++f) {
    for (uint32_t start = 0; start < size_to_test / 2; ++start) {
      ++TotalTests;
      uint32_t inc = (start / 10) + 1;
      uint32_t llc_result = Funcs[f].FuncLlc(size_to_test, start, inc);
      uint32_t sz_result = Funcs[f].FuncSz(size_to_test, start, inc);
      if (llc_result == sz_result) {
        ++Passes;
      } else {
        ++Failures;
        printf("Failure %s: start=%" PRIu32 ", "
               "llc=%" PRIu32 ", sz=%" PRIu32 "\n",
               Funcs[f].Name, start, llc_result, sz_result);
      }
    }
  }
  printf("TotalTests=%zu Passes=%zu Failures=%zu\n", TotalTests, Passes,
         Failures);
  return Failures;
}
