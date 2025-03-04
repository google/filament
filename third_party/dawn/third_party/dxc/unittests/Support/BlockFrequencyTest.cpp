//===- unittests/Support/BlockFrequencyTest.cpp - BlockFrequency tests ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/BlockFrequency.h"
#include "llvm/Support/BranchProbability.h"
#include "llvm/Support/DataTypes.h"
#include "gtest/gtest.h"
#include <climits>

using namespace llvm;

namespace {

TEST(BlockFrequencyTest, OneToZero) {
  BlockFrequency Freq(1);
  BranchProbability Prob(UINT32_MAX - 1, UINT32_MAX);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), 0u);

  Freq = BlockFrequency(1);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), 0u);
}

TEST(BlockFrequencyTest, OneToOne) {
  BlockFrequency Freq(1);
  BranchProbability Prob(UINT32_MAX, UINT32_MAX);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), 1u);

  Freq = BlockFrequency(1);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), 1u);
}

TEST(BlockFrequencyTest, ThreeToOne) {
  BlockFrequency Freq(3);
  BranchProbability Prob(3000000, 9000000);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), 1u);

  Freq = BlockFrequency(3);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), 1u);
}

TEST(BlockFrequencyTest, MaxToHalfMax) {
  BlockFrequency Freq(UINT64_MAX);
  BranchProbability Prob(UINT32_MAX / 2, UINT32_MAX);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), 9223372034707292159ULL);

  Freq = BlockFrequency(UINT64_MAX);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), 9223372034707292159ULL);
}

TEST(BlockFrequencyTest, BigToBig) {
  const uint64_t Big = 387246523487234346LL;
  const uint32_t P = 123456789;
  BlockFrequency Freq(Big);
  BranchProbability Prob(P, P);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), Big);

  Freq = BlockFrequency(Big);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), Big);
}

TEST(BlockFrequencyTest, MaxToMax) {
  BlockFrequency Freq(UINT64_MAX);
  BranchProbability Prob(UINT32_MAX, UINT32_MAX);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), UINT64_MAX);

  // This additionally makes sure if we have a value equal to our saturating
  // value, we do not signal saturation if the result equals said value, but
  // saturating does not occur.
  Freq = BlockFrequency(UINT64_MAX);
  Freq *= Prob;
  EXPECT_EQ(Freq.getFrequency(), UINT64_MAX);
}

TEST(BlockFrequency, Divide) {
  BlockFrequency Freq(0x3333333333333333ULL);
  Freq /= BranchProbability(1, 2);
  EXPECT_EQ(Freq.getFrequency(), 0x6666666666666666ULL);
}

TEST(BlockFrequencyTest, Saturate) {
  BlockFrequency Freq(0x3333333333333333ULL);
  Freq /= BranchProbability(100, 300);
  EXPECT_EQ(Freq.getFrequency(), 0x9999999999999999ULL);
  Freq /= BranchProbability(1, 2);
  EXPECT_EQ(Freq.getFrequency(), UINT64_MAX);

  Freq = 0x1000000000000000ULL;
  Freq /= BranchProbability(10000, 160000);
  EXPECT_EQ(Freq.getFrequency(), UINT64_MAX);

  // Try to cheat the multiplication overflow check.
  Freq = 0x00000001f0000001ull;
  Freq /= BranchProbability(1000, 0xf000000f);
  EXPECT_EQ(33506781356485509ULL, Freq.getFrequency());
}

TEST(BlockFrequencyTest, SaturatingRightShift) {
  BlockFrequency Freq(0x10080ULL);
  Freq >>= 2;
  EXPECT_EQ(Freq.getFrequency(), 0x4020ULL);
  Freq >>= 20;
  EXPECT_EQ(Freq.getFrequency(), 0x1ULL);
}

}
