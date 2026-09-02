//===- unittests/SPIRV/SpirvConstantTest.cpp --- SPIR-V Constant tests ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "SpirvTestBase.h"
#include "clang/SPIRV/SpirvInstruction.h"

using namespace clang::spirv;

namespace {

class SpirvConstantTest : public SpirvTestBase {};

TEST_F(SpirvConstantTest, BoolFalse) {
  clang::ASTContext &astContext = getAstContext();
  const bool val = false;
  SpirvConstantBoolean constant(astContext.BoolTy, val);
  EXPECT_EQ(val, constant.getValue());
}

TEST_F(SpirvConstantTest, BoolTrue) {
  clang::ASTContext &astContext = getAstContext();
  const bool val = true;
  SpirvConstantBoolean constant(astContext.BoolTy, val);
  EXPECT_EQ(val, constant.getValue());
}

TEST_F(SpirvConstantTest, Uint16) {
  clang::ASTContext &astContext = getAstContext();
  const auto u16 = llvm::APInt(16, 12u);
  SpirvConstantInteger constant(astContext.UnsignedShortTy, u16);
  EXPECT_EQ(u16, constant.getValue());
}

TEST_F(SpirvConstantTest, Int16) {
  clang::ASTContext &astContext = getAstContext();
  const auto i16 = llvm::APInt(16, -12, /*isSigned*/ true);
  SpirvConstantInteger constant(astContext.ShortTy, i16);
  EXPECT_EQ(i16, constant.getValue());
}

TEST_F(SpirvConstantTest, Uint32) {
  clang::ASTContext &astContext = getAstContext();
  const auto u32 = llvm::APInt(32, 65536);
  SpirvConstantInteger constant(astContext.UnsignedIntTy, u32);
  EXPECT_EQ(u32, constant.getValue());
}

TEST_F(SpirvConstantTest, Int32) {
  clang::ASTContext &astContext = getAstContext();
  const auto i32 = llvm::APInt(32, -65536, /*isSigned*/ true);
  SpirvConstantInteger constant(astContext.IntTy, i32);
  EXPECT_EQ(i32, constant.getValue());
}

TEST_F(SpirvConstantTest, Uint64) {
  clang::ASTContext &astContext = getAstContext();
  const auto u64 = llvm::APInt(64, 4294967296);
  SpirvConstantInteger constant(astContext.UnsignedLongLongTy, u64);
  EXPECT_EQ(u64, constant.getValue());
}

TEST_F(SpirvConstantTest, Int64) {
  clang::ASTContext &astContext = getAstContext();
  const auto i64 = llvm::APInt(64, -4294967296, /*isSigned*/ true);
  SpirvConstantInteger constant(astContext.LongLongTy, i64);
  EXPECT_EQ(i64, constant.getValue());
}

TEST_F(SpirvConstantTest, Float32) {
  clang::ASTContext &astContext = getAstContext();
  const auto f32 = llvm::APFloat(1.5f);
  SpirvConstantFloat constant(astContext.FloatTy, f32);
  EXPECT_EQ(1.5f, constant.getValue().convertToFloat());
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnBool) {
  clang::ASTContext &astContext = getAstContext();
  const bool val = true;
  SpirvConstantBoolean constant1(astContext.BoolTy, val);
  SpirvConstantBoolean constant2(astContext.BoolTy, val);
  EXPECT_TRUE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnInt) {
  clang::ASTContext &astContext = getAstContext();
  const auto i32 = llvm::APInt(32, -65536, /*isSigned*/ true);
  SpirvConstantInteger constant1(astContext.IntTy, i32);
  SpirvConstantInteger constant2(astContext.IntTy, i32);
  EXPECT_TRUE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnFloat) {
  clang::ASTContext &astContext = getAstContext();
  const auto f32 = llvm::APFloat(1.5f);
  SpirvConstantFloat constant1(astContext.FloatTy, f32);
  SpirvConstantFloat constant2(astContext.FloatTy, f32);
  EXPECT_TRUE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnNull) {
  clang::ASTContext &astContext = getAstContext();
  SpirvConstantNull constant1(astContext.IntTy);
  SpirvConstantNull constant2(astContext.IntTy);
  EXPECT_TRUE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnBool2) {
  clang::ASTContext &astContext = getAstContext();
  SpirvConstantBoolean constant1(astContext.BoolTy, true);
  SpirvConstantBoolean constant2(astContext.BoolTy, false);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnInt2) {
  clang::ASTContext &astContext = getAstContext();
  const auto i1 = llvm::APInt(32, 5, /*isSigned*/ true);
  const auto i2 = llvm::APInt(32, 7, /*isSigned*/ true);
  SpirvConstantInteger constant1(astContext.IntTy, i1);
  SpirvConstantInteger constant2(astContext.IntTy, i2);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnFloat2) {
  clang::ASTContext &astContext = getAstContext();
  const auto f1 = llvm::APFloat(1.5f);
  const auto f2 = llvm::APFloat(1.6f);
  SpirvConstantFloat constant1(astContext.FloatTy, f1);
  SpirvConstantFloat constant2(astContext.FloatTy, f2);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnInt3) {
  // Different signedness should mean different constants.
  clang::ASTContext &astContext = getAstContext();
  const auto i32 = llvm::APInt(32, 7, /*isSigned*/ true);
  SpirvConstantInteger constant1(astContext.UnsignedIntTy, i32);
  SpirvConstantInteger constant2(astContext.IntTy, i32);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnFloat3) {
  // Different bitwidth should mean different constants.
  clang::ASTContext &astContext = getAstContext();
  const auto f32 = llvm::APFloat(1.5f);
  SpirvConstantFloat constant1(astContext.DoubleTy, f32);
  SpirvConstantFloat constant2(astContext.FloatTy, f32);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnInt4) {
  // Different bitwidth should mean different constants.
  clang::ASTContext &astContext = getAstContext();
  const auto i32 = llvm::APInt(32, 7, /*isSigned*/ true);
  SpirvConstantInteger constant1(astContext.ShortTy, i32);
  SpirvConstantInteger constant2(astContext.IntTy, i32);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, CheckOperatorEqualOnNull2) {
  clang::ASTContext &astContext = getAstContext();
  SpirvConstantNull constant1(astContext.IntTy);
  SpirvConstantNull constant2(astContext.UnsignedIntTy);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, BoolConstNotEqualSpecConst) {
  clang::ASTContext &astContext = getAstContext();
  SpirvConstantBoolean constant1(astContext.BoolTy, true, /*SpecConst*/ true);
  SpirvConstantBoolean constant2(astContext.BoolTy, false, /*SpecConst*/ false);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, IntConstNotEqualSpecConst) {
  clang::ASTContext &astContext = getAstContext();
  const auto i32 = llvm::APInt(32, 7, /*isSigned*/ true);
  SpirvConstantInteger constant1(astContext.IntTy, i32, /*SpecConst*/ false);
  SpirvConstantInteger constant2(astContext.IntTy, i32, /*SpecConst*/ true);
  EXPECT_FALSE(constant1 == constant2);
}

TEST_F(SpirvConstantTest, FloatConstNotEqualSpecConst) {
  clang::ASTContext &astContext = getAstContext();
  const auto f32 = llvm::APFloat(1.5f);
  SpirvConstantFloat constant1(astContext.FloatTy, f32, /*SpecConst*/ false);
  SpirvConstantFloat constant2(astContext.FloatTy, f32, /*SpecConst*/ true);
  EXPECT_FALSE(constant1 == constant2);
}

} // anonymous namespace
