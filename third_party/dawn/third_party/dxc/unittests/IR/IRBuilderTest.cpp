//===- llvm/unittest/IR/IRBuilderTest.cpp - IRBuilder tests ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/NoFolder.h"
#include "llvm/IR/Verifier.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

class IRBuilderTest : public testing::Test {
protected:
  void SetUp() override {
    M.reset(new Module("MyModule", Ctx));
    FunctionType *FTy = FunctionType::get(Type::getVoidTy(Ctx),
                                          /*isVarArg=*/false);
    F = Function::Create(FTy, Function::ExternalLinkage, "", M.get());
    BB = BasicBlock::Create(Ctx, "", F);
    GV = new GlobalVariable(*M, Type::getFloatTy(Ctx), true,
                            GlobalValue::ExternalLinkage, nullptr);
  }

  void TearDown() override {
    BB = nullptr;
    M.reset();
  }

  LLVMContext Ctx;
  std::unique_ptr<Module> M;
  Function *F;
  BasicBlock *BB;
  GlobalVariable *GV;
};

TEST_F(IRBuilderTest, Lifetime) {
  IRBuilder<> Builder(BB);
  AllocaInst *Var1 = Builder.CreateAlloca(Builder.getInt8Ty());
  AllocaInst *Var2 = Builder.CreateAlloca(Builder.getInt32Ty());
  AllocaInst *Var3 = Builder.CreateAlloca(Builder.getInt8Ty(),
                                          Builder.getInt32(123));

  CallInst *Start1 = Builder.CreateLifetimeStart(Var1);
  CallInst *Start2 = Builder.CreateLifetimeStart(Var2);
  CallInst *Start3 = Builder.CreateLifetimeStart(Var3, Builder.getInt64(100));

  EXPECT_EQ(Start1->getArgOperand(0), Builder.getInt64(-1));
  EXPECT_EQ(Start2->getArgOperand(0), Builder.getInt64(-1));
  EXPECT_EQ(Start3->getArgOperand(0), Builder.getInt64(100));

  EXPECT_EQ(Start1->getArgOperand(1), Var1);
  EXPECT_NE(Start2->getArgOperand(1), Var2);
  EXPECT_EQ(Start3->getArgOperand(1), Var3);

  Value *End1 = Builder.CreateLifetimeEnd(Var1);
  Builder.CreateLifetimeEnd(Var2);
  Builder.CreateLifetimeEnd(Var3);

  IntrinsicInst *II_Start1 = dyn_cast<IntrinsicInst>(Start1);
  IntrinsicInst *II_End1 = dyn_cast<IntrinsicInst>(End1);
  ASSERT_TRUE(II_Start1 != nullptr);
  EXPECT_EQ(II_Start1->getIntrinsicID(), Intrinsic::lifetime_start);
  ASSERT_TRUE(II_End1 != nullptr);
  EXPECT_EQ(II_End1->getIntrinsicID(), Intrinsic::lifetime_end);
}

TEST_F(IRBuilderTest, CreateCondBr) {
  IRBuilder<> Builder(BB);
  BasicBlock *TBB = BasicBlock::Create(Ctx, "", F);
  BasicBlock *FBB = BasicBlock::Create(Ctx, "", F);

  BranchInst *BI = Builder.CreateCondBr(Builder.getTrue(), TBB, FBB);
  TerminatorInst *TI = BB->getTerminator();
  EXPECT_EQ(BI, TI);
  EXPECT_EQ(2u, TI->getNumSuccessors());
  EXPECT_EQ(TBB, TI->getSuccessor(0));
  EXPECT_EQ(FBB, TI->getSuccessor(1));

  BI->eraseFromParent();
  MDNode *Weights = MDBuilder(Ctx).createBranchWeights(42, 13);
  BI = Builder.CreateCondBr(Builder.getTrue(), TBB, FBB, Weights);
  TI = BB->getTerminator();
  EXPECT_EQ(BI, TI);
  EXPECT_EQ(2u, TI->getNumSuccessors());
  EXPECT_EQ(TBB, TI->getSuccessor(0));
  EXPECT_EQ(FBB, TI->getSuccessor(1));
  EXPECT_EQ(Weights, TI->getMetadata(LLVMContext::MD_prof));
}

TEST_F(IRBuilderTest, LandingPadName) {
  IRBuilder<> Builder(BB);
  LandingPadInst *LP = Builder.CreateLandingPad(Builder.getInt32Ty(), 0, "LP");
  EXPECT_EQ(LP->getName(), "LP");
}

TEST_F(IRBuilderTest, DataLayout) {
  std::unique_ptr<Module> M(new Module("test", Ctx));
  M->setDataLayout("e-n32");
  EXPECT_TRUE(M->getDataLayout().isLegalInteger(32));
  M->setDataLayout("e");
  EXPECT_FALSE(M->getDataLayout().isLegalInteger(32));
}

TEST_F(IRBuilderTest, GetIntTy) {
  IRBuilder<> Builder(BB);
  IntegerType *Ty1 = Builder.getInt1Ty();
  EXPECT_EQ(Ty1, IntegerType::get(Ctx, 1));

  DataLayout* DL = new DataLayout(M.get());
  IntegerType *IntPtrTy = Builder.getIntPtrTy(*DL);
  unsigned IntPtrBitSize =  DL->getPointerSizeInBits(0);
  EXPECT_EQ(IntPtrTy, IntegerType::get(Ctx, IntPtrBitSize));
  delete DL;
}

TEST_F(IRBuilderTest, FastMathFlags) {
  IRBuilder<> Builder(BB);
  Value *F, *FC;
  Instruction *FDiv, *FAdd, *FCmp;

  F = Builder.CreateLoad(GV);
  F = Builder.CreateFAdd(F, F);

  EXPECT_FALSE(Builder.getFastMathFlags().any());
  ASSERT_TRUE(isa<Instruction>(F));
  FAdd = cast<Instruction>(F);
  EXPECT_FALSE(FAdd->hasNoNaNs());

  FastMathFlags FMF;
  Builder.SetFastMathFlags(FMF);

  F = Builder.CreateFAdd(F, F);
  EXPECT_FALSE(Builder.getFastMathFlags().any());

  FMF.setUnsafeAlgebra();
  Builder.SetFastMathFlags(FMF);

  F = Builder.CreateFAdd(F, F);
  EXPECT_TRUE(Builder.getFastMathFlags().any());
  ASSERT_TRUE(isa<Instruction>(F));
  FAdd = cast<Instruction>(F);
  EXPECT_TRUE(FAdd->hasNoNaNs());

  // Now, try it with CreateBinOp
  F = Builder.CreateBinOp(Instruction::FAdd, F, F);
  EXPECT_TRUE(Builder.getFastMathFlags().any());
  ASSERT_TRUE(isa<Instruction>(F));
  FAdd = cast<Instruction>(F);
  EXPECT_TRUE(FAdd->hasNoNaNs());

  F = Builder.CreateFDiv(F, F);
  EXPECT_TRUE(Builder.getFastMathFlags().any());
  EXPECT_TRUE(Builder.getFastMathFlags().UnsafeAlgebra);
  ASSERT_TRUE(isa<Instruction>(F));
  FDiv = cast<Instruction>(F);
  EXPECT_TRUE(FDiv->hasAllowReciprocal());

  Builder.clearFastMathFlags();

  F = Builder.CreateFDiv(F, F);
  ASSERT_TRUE(isa<Instruction>(F));
  FDiv = cast<Instruction>(F);
  EXPECT_FALSE(FDiv->hasAllowReciprocal());

  FMF.clear();
  FMF.setAllowReciprocal();
  Builder.SetFastMathFlags(FMF);

  F = Builder.CreateFDiv(F, F);
  EXPECT_TRUE(Builder.getFastMathFlags().any());
  EXPECT_TRUE(Builder.getFastMathFlags().AllowReciprocal);
  ASSERT_TRUE(isa<Instruction>(F));
  FDiv = cast<Instruction>(F);
  EXPECT_TRUE(FDiv->hasAllowReciprocal());

  Builder.clearFastMathFlags();

  FC = Builder.CreateFCmpOEQ(F, F);
  ASSERT_TRUE(isa<Instruction>(FC));
  FCmp = cast<Instruction>(FC);
  EXPECT_FALSE(FCmp->hasAllowReciprocal());

  FMF.clear();
  FMF.setAllowReciprocal();
  Builder.SetFastMathFlags(FMF);

  FC = Builder.CreateFCmpOEQ(F, F);
  EXPECT_TRUE(Builder.getFastMathFlags().any());
  EXPECT_TRUE(Builder.getFastMathFlags().AllowReciprocal);
  ASSERT_TRUE(isa<Instruction>(FC));
  FCmp = cast<Instruction>(FC);
  EXPECT_TRUE(FCmp->hasAllowReciprocal());

  Builder.clearFastMathFlags();

  // To test a copy, make sure that a '0' and a '1' change state. 
  F = Builder.CreateFDiv(F, F);
  ASSERT_TRUE(isa<Instruction>(F));
  FDiv = cast<Instruction>(F);
  EXPECT_FALSE(FDiv->getFastMathFlags().any());
  FDiv->setHasAllowReciprocal(true);
  FAdd->setHasAllowReciprocal(false);
  FDiv->copyFastMathFlags(FAdd);
  EXPECT_TRUE(FDiv->hasNoNaNs());
  EXPECT_FALSE(FDiv->hasAllowReciprocal());

}

TEST_F(IRBuilderTest, WrapFlags) {
  IRBuilder<true, NoFolder> Builder(BB);

  // Test instructions.
  GlobalVariable *G = new GlobalVariable(*M, Builder.getInt32Ty(), true,
                                         GlobalValue::ExternalLinkage, nullptr);
  Value *V = Builder.CreateLoad(G);
  EXPECT_TRUE(
      cast<BinaryOperator>(Builder.CreateNSWAdd(V, V))->hasNoSignedWrap());
  EXPECT_TRUE(
      cast<BinaryOperator>(Builder.CreateNSWMul(V, V))->hasNoSignedWrap());
  EXPECT_TRUE(
      cast<BinaryOperator>(Builder.CreateNSWSub(V, V))->hasNoSignedWrap());
  EXPECT_TRUE(cast<BinaryOperator>(
                  Builder.CreateShl(V, V, "", /* NUW */ false, /* NSW */ true))
                  ->hasNoSignedWrap());

  EXPECT_TRUE(
      cast<BinaryOperator>(Builder.CreateNUWAdd(V, V))->hasNoUnsignedWrap());
  EXPECT_TRUE(
      cast<BinaryOperator>(Builder.CreateNUWMul(V, V))->hasNoUnsignedWrap());
  EXPECT_TRUE(
      cast<BinaryOperator>(Builder.CreateNUWSub(V, V))->hasNoUnsignedWrap());
  EXPECT_TRUE(cast<BinaryOperator>(
                  Builder.CreateShl(V, V, "", /* NUW */ true, /* NSW */ false))
                  ->hasNoUnsignedWrap());

  // Test operators created with constants.
  Constant *C = Builder.getInt32(42);
  EXPECT_TRUE(cast<OverflowingBinaryOperator>(Builder.CreateNSWAdd(C, C))
                  ->hasNoSignedWrap());
  EXPECT_TRUE(cast<OverflowingBinaryOperator>(Builder.CreateNSWSub(C, C))
                  ->hasNoSignedWrap());
  EXPECT_TRUE(cast<OverflowingBinaryOperator>(Builder.CreateNSWMul(C, C))
                  ->hasNoSignedWrap());
  EXPECT_TRUE(cast<OverflowingBinaryOperator>(
                  Builder.CreateShl(C, C, "", /* NUW */ false, /* NSW */ true))
                  ->hasNoSignedWrap());

  EXPECT_TRUE(cast<OverflowingBinaryOperator>(Builder.CreateNUWAdd(C, C))
                  ->hasNoUnsignedWrap());
  EXPECT_TRUE(cast<OverflowingBinaryOperator>(Builder.CreateNUWSub(C, C))
                  ->hasNoUnsignedWrap());
  EXPECT_TRUE(cast<OverflowingBinaryOperator>(Builder.CreateNUWMul(C, C))
                  ->hasNoUnsignedWrap());
  EXPECT_TRUE(cast<OverflowingBinaryOperator>(
                  Builder.CreateShl(C, C, "", /* NUW */ true, /* NSW */ false))
                  ->hasNoUnsignedWrap());
}

TEST_F(IRBuilderTest, RAIIHelpersTest) {
  IRBuilder<> Builder(BB);
  EXPECT_FALSE(Builder.getFastMathFlags().allowReciprocal());
  MDBuilder MDB(M->getContext());

  MDNode *FPMathA = MDB.createFPMath(0.01f);
  MDNode *FPMathB = MDB.createFPMath(0.1f);

  Builder.SetDefaultFPMathTag(FPMathA);

  {
    IRBuilder<>::FastMathFlagGuard Guard(Builder);
    FastMathFlags FMF;
    FMF.setAllowReciprocal();
    Builder.SetFastMathFlags(FMF);
    Builder.SetDefaultFPMathTag(FPMathB);
    EXPECT_TRUE(Builder.getFastMathFlags().allowReciprocal());
    EXPECT_EQ(FPMathB, Builder.getDefaultFPMathTag());
  }

  EXPECT_FALSE(Builder.getFastMathFlags().allowReciprocal());
  EXPECT_EQ(FPMathA, Builder.getDefaultFPMathTag());

  Value *F = Builder.CreateLoad(GV);

  {
    IRBuilder<>::InsertPointGuard Guard(Builder);
    Builder.SetInsertPoint(cast<Instruction>(F));
    EXPECT_EQ(F, Builder.GetInsertPoint());
  }

  EXPECT_EQ(BB->end(), Builder.GetInsertPoint());
  EXPECT_EQ(BB, Builder.GetInsertBlock());
}

TEST_F(IRBuilderTest, DIBuilder) {
  IRBuilder<> Builder(BB);
  DIBuilder DIB(*M);
  auto File = DIB.createFile("F.CBL", "/");
  auto CU = DIB.createCompileUnit(dwarf::DW_LANG_Cobol74, "F.CBL", "/",
                                  "llvm-cobol74", true, "", 0);
  auto Type = DIB.createSubroutineType(File, DIB.getOrCreateTypeArray(None));
  DIB.createFunction(CU, "foo", "", File, 1, Type, false, true, 1, 0, true, F);
  AllocaInst *I = Builder.CreateAlloca(Builder.getInt8Ty());
  auto BarSP = DIB.createFunction(CU, "bar", "", File, 1, Type, false, true, 1,
                                  0, true, nullptr);
  auto BadScope = DIB.createLexicalBlockFile(BarSP, File, 0);
  I->setDebugLoc(DebugLoc::get(2, 0, BadScope));
  DIB.finalize();
  EXPECT_TRUE(verifyModule(*M));
}

TEST_F(IRBuilderTest, InsertExtractElement) {
  IRBuilder<> Builder(BB);

  auto VecTy = VectorType::get(Builder.getInt64Ty(), 4);
  auto Elt1 = Builder.getInt64(-1);
  auto Elt2 = Builder.getInt64(-2);
  Value *Vec = UndefValue::get(VecTy);
  Vec = Builder.CreateInsertElement(Vec, Elt1, Builder.getInt8(1));
  Vec = Builder.CreateInsertElement(Vec, Elt2, 2);
  auto X1 = Builder.CreateExtractElement(Vec, 1);
  auto X2 = Builder.CreateExtractElement(Vec, Builder.getInt32(2));
  EXPECT_EQ(Elt1, X1);
  EXPECT_EQ(Elt2, X2);
}

TEST_F(IRBuilderTest, CreateGlobalStringPtr) {
  IRBuilder<> Builder(BB);

  auto String1a = Builder.CreateGlobalStringPtr("TestString", "String1a");
  auto String1b = Builder.CreateGlobalStringPtr("TestString", "String1b", 0);
  auto String2 = Builder.CreateGlobalStringPtr("TestString", "String2", 1);
  auto String3 = Builder.CreateGlobalString("TestString", "String3", 2);

  EXPECT_TRUE(String1a->getType()->getPointerAddressSpace() == 0);
  EXPECT_TRUE(String1b->getType()->getPointerAddressSpace() == 0);
  EXPECT_TRUE(String2->getType()->getPointerAddressSpace() == 1);
  EXPECT_TRUE(String3->getType()->getPointerAddressSpace() == 2);
}

TEST_F(IRBuilderTest, DebugLoc) {
  auto CalleeTy = FunctionType::get(Type::getVoidTy(Ctx),
                                    /*isVarArg=*/false);
  auto Callee =
      Function::Create(CalleeTy, Function::ExternalLinkage, "", M.get());

  DIBuilder DIB(*M);
  auto File = DIB.createFile("tmp.cpp", "/");
  auto CU = DIB.createCompileUnit(dwarf::DW_LANG_C_plus_plus_11, "tmp.cpp", "/",
                                  "", true, "", 0);
  auto SPType = DIB.createSubroutineType(File, DIB.getOrCreateTypeArray(None));
  auto SP =
      DIB.createFunction(CU, "foo", "foo", File, 1, SPType, false, true, 1);
  DebugLoc DL1 = DILocation::get(Ctx, 2, 0, SP);
  DebugLoc DL2 = DILocation::get(Ctx, 3, 0, SP);

  auto BB2 = BasicBlock::Create(Ctx, "bb2", F);
  auto Br = BranchInst::Create(BB2, BB);
  Br->setDebugLoc(DL1);

  IRBuilder<> Builder(Ctx);
  Builder.SetInsertPoint(Br);
  EXPECT_EQ(DL1, Builder.getCurrentDebugLocation());
  auto Call1 = Builder.CreateCall(Callee, None);
  EXPECT_EQ(DL1, Call1->getDebugLoc());

  Call1->setDebugLoc(DL2);
  Builder.SetInsertPoint(Call1->getParent(), Call1);
  EXPECT_EQ(DL2, Builder.getCurrentDebugLocation());
  auto Call2 = Builder.CreateCall(Callee, None);
  EXPECT_EQ(DL2, Call2->getDebugLoc());

  DIB.finalize();
}
}
