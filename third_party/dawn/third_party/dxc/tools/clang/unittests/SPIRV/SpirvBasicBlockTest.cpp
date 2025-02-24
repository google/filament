//===- unittests/SPIRV/SpirvBasicBlockTest.cpp ----- Basic Block Tests ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/SPIRV/SpirvBasicBlock.h"
#include "clang/SPIRV/SpirvInstruction.h"

#include "SpirvTestBase.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace clang::spirv;

namespace {

class SpirvBasicBlockTest : public SpirvTestBase {};

TEST_F(SpirvBasicBlockTest, CheckName) {
  SpirvBasicBlock bb("myBasicBlock");
  EXPECT_EQ(bb.getName(), "myBasicBlock");
}

TEST_F(SpirvBasicBlockTest, CheckResultId) {
  SpirvBasicBlock bb("myBasicBlock");
  bb.setResultId(5);
  EXPECT_EQ(bb.getResultId(), 5u);
}

TEST_F(SpirvBasicBlockTest, CheckMergeTarget) {
  SpirvBasicBlock bb1("bb1");
  SpirvBasicBlock bb2("bb2");
  bb1.setMergeTarget(&bb2);
  EXPECT_EQ(bb1.getMergeTarget(), &bb2);
}

TEST_F(SpirvBasicBlockTest, CheckContinueTarget) {
  SpirvBasicBlock bb1("bb1");
  SpirvBasicBlock bb2("bb2");
  bb1.setContinueTarget(&bb2);
  EXPECT_EQ(bb1.getContinueTarget(), &bb2);
}

TEST_F(SpirvBasicBlockTest, CheckSuccessors) {
  SpirvBasicBlock bb1("bb1");
  SpirvBasicBlock bb2("bb2");
  SpirvBasicBlock bb3("bb3");
  bb1.addSuccessor(&bb2);
  bb1.addSuccessor(&bb3);
  auto successors = bb1.getSuccessors();
  EXPECT_EQ(successors[0], &bb2);
  EXPECT_EQ(successors[1], &bb3);
}

TEST_F(SpirvBasicBlockTest, CheckTerminatedByKill) {
  SpirvBasicBlock bb("bb");
  SpirvContext &context = getSpirvContext();
  auto *kill = new (context) SpirvKill({});
  bb.addInstruction(kill);
  EXPECT_TRUE(bb.hasTerminator());
}

TEST_F(SpirvBasicBlockTest, CheckTerminatedByBranch) {
  SpirvBasicBlock bb("bb");
  SpirvContext &context = getSpirvContext();
  auto *branch = new (context) SpirvBranch({}, nullptr);
  bb.addInstruction(branch);
  EXPECT_TRUE(bb.hasTerminator());
}

TEST_F(SpirvBasicBlockTest, CheckTerminatedByBranchConditional) {
  SpirvBasicBlock bb("bb");
  SpirvContext &context = getSpirvContext();
  auto *branch =
      new (context) SpirvBranchConditional({}, nullptr, nullptr, nullptr);
  bb.addInstruction(branch);
  EXPECT_TRUE(bb.hasTerminator());
}

TEST_F(SpirvBasicBlockTest, CheckTerminatedByReturn) {
  SpirvBasicBlock bb("bb");
  SpirvContext &context = getSpirvContext();
  auto *returnInstr = new (context) SpirvReturn({});
  bb.addInstruction(returnInstr);
  EXPECT_TRUE(bb.hasTerminator());
}

TEST_F(SpirvBasicBlockTest, CheckTerminatedByUnreachable) {
  SpirvBasicBlock bb("bb");
  SpirvContext &context = getSpirvContext();
  auto *unreachable = new (context) SpirvUnreachable({});
  bb.addInstruction(unreachable);
  EXPECT_TRUE(bb.hasTerminator());
}

TEST_F(SpirvBasicBlockTest, CheckTerminatedByTerminateRay) {
  SpirvBasicBlock bb("bb");
  SpirvContext &context = getSpirvContext();
  auto *khrTerminateRay = new (context)
      SpirvRayTracingTerminateOpKHR(spv::Op::OpTerminateRayKHR, {});
  bb.addInstruction(khrTerminateRay);
  EXPECT_TRUE(bb.hasTerminator());
}

TEST_F(SpirvBasicBlockTest, CheckTerminatedByIgnoreIntersection) {
  SpirvBasicBlock bb("bb");
  SpirvContext &context = getSpirvContext();
  auto *khrIgnoreIntersection = new (context)
      SpirvRayTracingTerminateOpKHR(spv::Op::OpIgnoreIntersectionKHR, {});
  bb.addInstruction(khrIgnoreIntersection);
  EXPECT_TRUE(bb.hasTerminator());
}

TEST_F(SpirvBasicBlockTest, CheckNotTerminated) {
  SpirvBasicBlock bb("bb");
  SpirvContext &context = getSpirvContext();
  auto *load = new (context) SpirvLoad({}, {}, nullptr);
  bb.addInstruction(load);
  EXPECT_FALSE(bb.hasTerminator());
}

} // anonymous namespace
