//===- llvm/unittest/ADT/MakeUniqueTest.cpp - make_unique unit tests ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/STLExtras.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

// Ensure that copies of a function_ref copy the underlying state rather than
// causing one function_ref to chain to the next.
TEST(FunctionRefTest, Copy) {
  auto A = [] { return 1; };
  auto B = [] { return 2; };
  function_ref<int()> X = A;
  function_ref<int()> Y = X;
  X = B;
  EXPECT_EQ(1, Y());
}

}
