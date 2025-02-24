//===- subzero/crosstest/test_calling_conv_main.cpp - Driver for tests ----===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the driver for cross testing the compatibility of
// calling conventions.
//
//===----------------------------------------------------------------------===//

/* crosstest.py --test=test_calling_conv.cpp               \
   --driver=test_calling_conv_main.cpp --prefix=Subzero_   \
   --output=test_calling_conv */

#include <cstring>
#include <iostream>
#include <sstream>

#include "test_calling_conv.h"

namespace Subzero_ {
#include "test_calling_conv.h"
}

// The crosstest code consists of caller / callee function pairs.
//
// The caller function initializes a list of arguments and calls the
// function located at Callee.
//
// The callee function writes the argument numbered ArgNum into the
// location pointed to by Buf.
//
// testCaller() tests that caller functions, as compiled by Subzero and
// llc, pass arguments to the callee in the same way.  The Caller() and
// Subzero_Caller() functions both call the same callee (which has been
// compiled by llc).  The result in the global buffer is compared to
// check that it is the same value after the calls by both callers.
//
// testCallee() runs the same kind of test, except that the functions
// Callee() and Subzero_Callee() are being tested to ensure that both
// functions receive arguments from the caller in the same way.  The
// caller is compiled by llc.

size_t ArgNum;
CalleePtrTy Callee;
char *Buf;

const static size_t BUF_SIZE = 16;

std::string bufAsString(const char Buf[BUF_SIZE]) {
  std::ostringstream OS;
  for (size_t i = 0; i < BUF_SIZE; ++i) {
    if (i > 0)
      OS << " ";
    OS << (unsigned)Buf[i];
  }
  return OS.str();
}

void testCaller(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  static struct {
    const char *CallerName, *CalleeName;
    size_t Args;
    void (*Caller)(void);
    void (*Subzero_Caller)(void);
    CalleePtrTy Callee;
  } Funcs[] = {
#ifdef MIPS32
#define X(caller, callee, argc)                                                \
  {                                                                            \
      STR(caller),                                                             \
      STR(callee),                                                             \
      argc,                                                                    \
      &caller,                                                                 \
      &Subzero_::caller,                                                       \
      reinterpret_cast<CalleePtrTy>(&Subzero_::callee),                        \
  },
      TEST_FUNC_TABLE
#undef X
#else
#define X(caller, callee, argc)                                                \
  {                                                                            \
      STR(caller), STR(callee),       argc,                                    \
      &caller,     &Subzero_::caller, reinterpret_cast<CalleePtrTy>(&callee),  \
  },
      TEST_FUNC_TABLE
#undef X
#endif
  };

  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);

  for (size_t f = 0; f < NumFuncs; ++f) {
    char BufLlc[BUF_SIZE], BufSz[BUF_SIZE];
    Callee = Funcs[f].Callee;

    for (size_t i = 0; i < Funcs[f].Args; ++i) {
      memset(BufLlc, 0xff, sizeof(BufLlc));
      memset(BufSz, 0xff, sizeof(BufSz));

      ArgNum = i;

      Buf = BufLlc;
      Funcs[f].Caller();

      Buf = BufSz;
      Funcs[f].Subzero_Caller();

      ++TotalTests;
      if (!memcmp(BufLlc, BufSz, sizeof(BufLlc))) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "testCaller(Caller=" << Funcs[f].CallerName
                  << ", Callee=" << Funcs[f].CalleeName << ", ArgNum=" << ArgNum
                  << ")\nsz =" << bufAsString(BufSz)
                  << "\nllc=" << bufAsString(BufLlc) << "\n";
      }
    }
  }
}

void testCallee(size_t &TotalTests, size_t &Passes, size_t &Failures) {
  static struct {
    const char *CallerName, *CalleeName;
    size_t Args;
    void (*Caller)(void);
    CalleePtrTy Callee, Subzero_Callee;
  } Funcs[] = {
#define X(caller, callee, argc)                                                \
  {STR(caller),                                                                \
   STR(callee),                                                                \
   argc,                                                                       \
   &caller,                                                                    \
   reinterpret_cast<CalleePtrTy>(&callee),                                     \
   reinterpret_cast<CalleePtrTy>(&Subzero_::callee)},
      TEST_FUNC_TABLE
#undef X
  };

  const static size_t NumFuncs = sizeof(Funcs) / sizeof(*Funcs);

  for (size_t f = 0; f < NumFuncs; ++f) {
    char BufLlc[BUF_SIZE], BufSz[BUF_SIZE];

    for (size_t i = 0; i < Funcs[f].Args; ++i) {
      memset(BufLlc, 0xff, sizeof(BufLlc));
      memset(BufSz, 0xff, sizeof(BufSz));

      ArgNum = i;

      Buf = BufLlc;
      Callee = Funcs[f].Callee;
      Funcs[f].Caller();

      Buf = BufSz;
      Callee = Funcs[f].Subzero_Callee;
      Funcs[f].Caller();

      ++TotalTests;
      if (!memcmp(BufLlc, BufSz, sizeof(BufLlc))) {
        ++Passes;
      } else {
        ++Failures;
        std::cout << "testCallee(Caller=" << Funcs[f].CallerName
                  << ", Callee=" << Funcs[f].CalleeName << ", ArgNum=" << ArgNum
                  << ")\nsz =" << bufAsString(BufSz)
                  << "\nllc=" << bufAsString(BufLlc) << "\n";
      }
    }
  }
}

int main(int argc, char *argv[]) {
  size_t TotalTests = 0;
  size_t Passes = 0;
  size_t Failures = 0;

  testCaller(TotalTests, Passes, Failures);
  testCallee(TotalTests, Passes, Failures);

  std::cout << "TotalTests=" << TotalTests << " Passes=" << Passes
            << " Failures=" << Failures << "\n";

  return Failures;
}
