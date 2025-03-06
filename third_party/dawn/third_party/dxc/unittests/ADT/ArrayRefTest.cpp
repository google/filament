//===- llvm/unittest/ADT/ArrayRefTest.cpp - ArrayRef unit tests -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/ArrayRef.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/raw_ostream.h"
#include "gtest/gtest.h"
#include <vector>
using namespace llvm;

// Check that the ArrayRef-of-pointer converting constructor only allows adding
// cv qualifiers (not removing them, or otherwise changing the type)
static_assert(
    std::is_convertible<ArrayRef<int *>, ArrayRef<const int *>>::value,
    "Adding const");
static_assert(
    std::is_convertible<ArrayRef<int *>, ArrayRef<volatile int *>>::value,
    "Adding volatile");
static_assert(!std::is_convertible<ArrayRef<int *>, ArrayRef<float *>>::value,
              "Changing pointer of one type to a pointer of another");
static_assert(
    !std::is_convertible<ArrayRef<const int *>, ArrayRef<int *>>::value,
    "Removing const");
static_assert(
    !std::is_convertible<ArrayRef<volatile int *>, ArrayRef<int *>>::value,
    "Removing volatile");

namespace llvm {

TEST(ArrayRefTest, AllocatorCopy) {
  BumpPtrAllocator Alloc;
  static const uint16_t Words1[] = { 1, 4, 200, 37 };
  ArrayRef<uint16_t> Array1 = makeArrayRef(Words1, 4);
  static const uint16_t Words2[] = { 11, 4003, 67, 64000, 13 };
  ArrayRef<uint16_t> Array2 = makeArrayRef(Words2, 5);
  ArrayRef<uint16_t> Array1c = Array1.copy(Alloc);
  ArrayRef<uint16_t> Array2c = Array2.copy(Alloc);
  EXPECT_TRUE(Array1.equals(Array1c));
  EXPECT_NE(Array1.data(), Array1c.data());
  EXPECT_TRUE(Array2.equals(Array2c));
  EXPECT_NE(Array2.data(), Array2c.data());
}

TEST(ArrayRefTest, DropBack) {
  static const int TheNumbers[] = {4, 8, 15, 16, 23, 42};
  ArrayRef<int> AR1(TheNumbers);
  ArrayRef<int> AR2(TheNumbers, AR1.size() - 1);
  EXPECT_TRUE(AR1.drop_back().equals(AR2));
}

TEST(ArrayRefTest, Equals) {
  static const int A1[] = {1, 2, 3, 4, 5, 6, 7, 8};
  ArrayRef<int> AR1(A1);
  EXPECT_TRUE(AR1.equals({1, 2, 3, 4, 5, 6, 7, 8}));
  EXPECT_FALSE(AR1.equals({8, 1, 2, 4, 5, 6, 6, 7}));
  EXPECT_FALSE(AR1.equals({2, 4, 5, 6, 6, 7, 8, 1}));
  EXPECT_FALSE(AR1.equals({0, 1, 2, 4, 5, 6, 6, 7}));
  EXPECT_FALSE(AR1.equals({1, 2, 42, 4, 5, 6, 7, 8}));
  EXPECT_FALSE(AR1.equals({42, 2, 3, 4, 5, 6, 7, 8}));
  EXPECT_FALSE(AR1.equals({1, 2, 3, 4, 5, 6, 7, 42}));
  EXPECT_FALSE(AR1.equals({1, 2, 3, 4, 5, 6, 7}));
  EXPECT_FALSE(AR1.equals({1, 2, 3, 4, 5, 6, 7, 8, 9}));

  ArrayRef<int> AR1a = AR1.drop_back();
  EXPECT_TRUE(AR1a.equals({1, 2, 3, 4, 5, 6, 7}));
  EXPECT_FALSE(AR1a.equals({1, 2, 3, 4, 5, 6, 7, 8}));

  ArrayRef<int> AR1b = AR1a.slice(2, 4);
  EXPECT_TRUE(AR1b.equals({3, 4, 5, 6}));
  EXPECT_FALSE(AR1b.equals({2, 3, 4, 5, 6}));
  EXPECT_FALSE(AR1b.equals({3, 4, 5, 6, 7}));
}

TEST(ArrayRefTest, EmptyEquals) {
  EXPECT_TRUE(ArrayRef<unsigned>() == ArrayRef<unsigned>());
}

TEST(ArrayRefTest, ConstConvert) {
  int buf[4];
  for (int i = 0; i < 4; ++i)
    buf[i] = i;

  static int *A[] = {&buf[0], &buf[1], &buf[2], &buf[3]};
  ArrayRef<const int *> a((ArrayRef<int *>(A)));
  a = ArrayRef<int *>(A);
}

static std::vector<int> ReturnTest12() { return {1, 2}; }
static void ArgTest12(ArrayRef<int> A) {
  EXPECT_EQ(2U, A.size());
  EXPECT_EQ(1, A[0]);
  EXPECT_EQ(2, A[1]);
}

TEST(ArrayRefTest, InitializerList) {
  std::initializer_list<int> InitList = { 0, 1, 2, 3, 4 };
  ArrayRef<int> A = InitList;
  for (int i = 0; i < 5; ++i)
    EXPECT_EQ(i, A[i]);

  std::vector<int> B = ReturnTest12();
  A = B;
  EXPECT_EQ(1, A[0]);
  EXPECT_EQ(2, A[1]);

  ArgTest12({1, 2});
}

} // end anonymous namespace
