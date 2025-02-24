//===- subzero/crosstest/test_bitmanip_main.cpp - Driver for tests. -------===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Driver for cross testing bit manipulation intrinsics.
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_bitmanip.cpp --test=test_bitmanip_intrin.ll \
   --driver=test_bitmanip_main.cpp --prefix=Subzero_ --output=test_bitmanip */

#include <stdint.h>

#include <climits>
#include <iostream>

// Include test_bitmanip.h twice - once normally, and once within the
// Subzero_ namespace, corresponding to the llc and Subzero translated
// object files, respectively.
#include "test_bitmanip.h"
#include "xdefs.h"

namespace Subzero_ {
#include "test_bitmanip.h"
}

volatile uint64 Values[] = {0,
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
                            0xc0de,
                            0xabcd,
                            0xdcba,
                            0x007fffff /*Max subnormal + */,
                            0x00800000 /*Min+ */,
                            0x7f7fffff /*Max+ */,
                            0x7f800000 /*+Inf*/,
                            0xff800000 /*-Inf*/,
                            0x7fa00000 /*SNaN*/,
                            0x7fc00000 /*QNaN*/,
                            0x7ffffffe,
                            0x7fffffff,
                            0x80000000,
                            0x80000001,
                            0xfffffffe,
                            0xffffffff,
                            0x12345678,
                            0xabcd1234,
                            0x1234dcba,
                            0x100000000ll,
                            0x100000001ll,
                            0x123456789abcdef1ll,
                            0x987654321ab1fedcll,
                            0x000fffffffffffffll /*Max subnormal + */,
                            0x0010000000000000ll /*Min+ */,
                            0x7fefffffffffffffll /*Max+ */,
                            0x7ff0000000000000ll /*+Inf*/,
                            0xfff0000000000000ll /*-Inf*/,
                            0x7ff0000000000001ll /*SNaN*/,
                            0x7ff8000000000000ll /*QNaN*/,
                            0x7ffffffffffffffell,
                            0x7fffffffffffffffll,
                            0x8000000000000000ll,
                            0x8000000000000001ll,
                            0xfffffffffffffffell,
                            0xffffffffffffffffll};

const static size_t NumValues = sizeof(Values) / sizeof(*Values);

template <typename Type>
void testBitManip(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef Type (*FuncType)(Type);
  static struct {
    const char *Name;
    FuncType FuncLlc;
    FuncType FuncSz;
  } Funcs[] = {
#define X(inst)                                                                \
  {STR(inst), test_##inst, Subzero_::test_##inst},                             \
      {STR(inst) "_alloca", test_alloca_##inst, Subzero_::test_alloca_##inst}, \
      {STR(inst) "_const", test_const_##inst, Subzero_::test_const_##inst},
      BMI_OPS
#undef X
  };
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);

  for (size_t f = 0; f < NumFuncs; ++f) {
    for (size_t i = 0; i < NumValues; ++i) {
      Type Value = static_cast<Type>(Values[i]);
      ++TotalTests;
      Type ResultSz = Funcs[f].FuncSz(Value);
      Type ResultLlc = Funcs[f].FuncLlc(Value);
      if (ResultSz == ResultLlc) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "test_" << Funcs[f].Name << (CHAR_BIT * sizeof(Type))
                  << "(" << static_cast<uint64>(Value)
                  << "): sz=" << static_cast<uint64>(ResultSz)
                  << " llc=" << static_cast<uint64>(ResultLlc) << "\n";
      }
    }
  }
}

template <typename Type>
void testByteSwap(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  typedef Type (*FuncType)(Type);
  static struct {
    const char *Name;
    FuncType FuncLlc;
    FuncType FuncSz;
  } Funcs[] = {
      {"bswap", test_bswap, Subzero_::test_bswap},
      {"bswap_alloca", test_bswap_alloca, Subzero_::test_bswap_alloca}};
  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);
  for (size_t f = 0; f < NumFuncs; ++f) {
    for (size_t i = 0; i < NumValues; ++i) {
      Type Value = static_cast<Type>(Values[i]);
      ++TotalTests;
      Type ResultSz = Funcs[f].FuncSz(Value);
      Type ResultLlc = Funcs[f].FuncLlc(Value);
      if (ResultSz == ResultLlc) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "test_" << Funcs[f].Name << (CHAR_BIT * sizeof(Type))
                  << "(" << static_cast<uint64>(Value)
                  << "): sz=" << static_cast<uint64>(ResultSz)
                  << " llc=" << static_cast<uint64>(ResultLlc) << "\n";
      }
    }
  }
}

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;

  testBitManip<uint32_t>(TotalTests, Passes, Failures);
  testBitManip<uint64>(TotalTests, Passes, Failures);
  testByteSwap<uint16_t>(TotalTests, Passes, Failures);
  testByteSwap<uint32_t>(TotalTests, Passes, Failures);
  testByteSwap<uint64>(TotalTests, Passes, Failures);

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";
  return Failures;
}
