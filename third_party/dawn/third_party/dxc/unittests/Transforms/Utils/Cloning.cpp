//===- Cloning.cpp - Unit tests for the Cloner ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "gtest/gtest.h"

using namespace llvm;

namespace {

class CloneInstruction : public ::testing::Test {
protected:
  void SetUp() override { V = nullptr; }

  template <typename T>
  T *clone(T *V1) {
    Value *V2 = V1->clone();
    Orig.insert(V1);
    Clones.insert(V2);
    return cast<T>(V2);
  }

  void eraseClones() {
    DeleteContainerPointers(Clones);
  }

  void TearDown() override {
    eraseClones();
    DeleteContainerPointers(Orig);
    delete V;
  }

  SmallPtrSet<Value *, 4> Orig;   // Erase on exit
  SmallPtrSet<Value *, 4> Clones; // Erase in eraseClones

  LLVMContext context;
  Value *V;
};

TEST_F(CloneInstruction, OverflowBits) {
  V = new Argument(Type::getInt32Ty(context));

  BinaryOperator *Add = BinaryOperator::Create(Instruction::Add, V, V);
  BinaryOperator *Sub = BinaryOperator::Create(Instruction::Sub, V, V);
  BinaryOperator *Mul = BinaryOperator::Create(Instruction::Mul, V, V);

  BinaryOperator *AddClone = this->clone(Add);
  BinaryOperator *SubClone = this->clone(Sub);
  BinaryOperator *MulClone = this->clone(Mul);

  EXPECT_FALSE(AddClone->hasNoUnsignedWrap());
  EXPECT_FALSE(AddClone->hasNoSignedWrap());
  EXPECT_FALSE(SubClone->hasNoUnsignedWrap());
  EXPECT_FALSE(SubClone->hasNoSignedWrap());
  EXPECT_FALSE(MulClone->hasNoUnsignedWrap());
  EXPECT_FALSE(MulClone->hasNoSignedWrap());

  eraseClones();

  Add->setHasNoUnsignedWrap();
  Sub->setHasNoUnsignedWrap();
  Mul->setHasNoUnsignedWrap();

  AddClone = this->clone(Add);
  SubClone = this->clone(Sub);
  MulClone = this->clone(Mul);

  EXPECT_TRUE(AddClone->hasNoUnsignedWrap());
  EXPECT_FALSE(AddClone->hasNoSignedWrap());
  EXPECT_TRUE(SubClone->hasNoUnsignedWrap());
  EXPECT_FALSE(SubClone->hasNoSignedWrap());
  EXPECT_TRUE(MulClone->hasNoUnsignedWrap());
  EXPECT_FALSE(MulClone->hasNoSignedWrap());

  eraseClones();

  Add->setHasNoSignedWrap();
  Sub->setHasNoSignedWrap();
  Mul->setHasNoSignedWrap();

  AddClone = this->clone(Add);
  SubClone = this->clone(Sub);
  MulClone = this->clone(Mul);

  EXPECT_TRUE(AddClone->hasNoUnsignedWrap());
  EXPECT_TRUE(AddClone->hasNoSignedWrap());
  EXPECT_TRUE(SubClone->hasNoUnsignedWrap());
  EXPECT_TRUE(SubClone->hasNoSignedWrap());
  EXPECT_TRUE(MulClone->hasNoUnsignedWrap());
  EXPECT_TRUE(MulClone->hasNoSignedWrap());

  eraseClones();

  Add->setHasNoUnsignedWrap(false);
  Sub->setHasNoUnsignedWrap(false);
  Mul->setHasNoUnsignedWrap(false);

  AddClone = this->clone(Add);
  SubClone = this->clone(Sub);
  MulClone = this->clone(Mul);

  EXPECT_FALSE(AddClone->hasNoUnsignedWrap());
  EXPECT_TRUE(AddClone->hasNoSignedWrap());
  EXPECT_FALSE(SubClone->hasNoUnsignedWrap());
  EXPECT_TRUE(SubClone->hasNoSignedWrap());
  EXPECT_FALSE(MulClone->hasNoUnsignedWrap());
  EXPECT_TRUE(MulClone->hasNoSignedWrap());
}

TEST_F(CloneInstruction, Inbounds) {
  V = new Argument(Type::getInt32PtrTy(context));

  Constant *Z = Constant::getNullValue(Type::getInt32Ty(context));
  std::vector<Value *> ops;
  ops.push_back(Z);
  GetElementPtrInst *GEP =
      GetElementPtrInst::Create(Type::getInt32Ty(context), V, ops);
  EXPECT_FALSE(this->clone(GEP)->isInBounds());

  GEP->setIsInBounds();
  EXPECT_TRUE(this->clone(GEP)->isInBounds());
}

TEST_F(CloneInstruction, Exact) {
  V = new Argument(Type::getInt32Ty(context));

  BinaryOperator *SDiv = BinaryOperator::Create(Instruction::SDiv, V, V);
  EXPECT_FALSE(this->clone(SDiv)->isExact());

  SDiv->setIsExact(true);
  EXPECT_TRUE(this->clone(SDiv)->isExact());
}

TEST_F(CloneInstruction, Attributes) {
  Type *ArgTy1[] = { Type::getInt32PtrTy(context) };
  FunctionType *FT1 =  FunctionType::get(Type::getVoidTy(context), ArgTy1, false);

  Function *F1 = Function::Create(FT1, Function::ExternalLinkage);
  BasicBlock *BB = BasicBlock::Create(context, "", F1);
  IRBuilder<> Builder(BB);
  Builder.CreateRetVoid();

  Function *F2 = Function::Create(FT1, Function::ExternalLinkage);

  Attribute::AttrKind AK[] = { Attribute::NoCapture };
  AttributeSet AS = AttributeSet::get(context, 0, AK);
  Argument *A = F1->arg_begin();
  A->addAttr(AS);

  SmallVector<ReturnInst*, 4> Returns;
  ValueToValueMapTy VMap;
  VMap[A] = UndefValue::get(A->getType());

  CloneFunctionInto(F2, F1, VMap, false, Returns);
  EXPECT_FALSE(F2->arg_begin()->hasNoCaptureAttr());

  delete F1;
  delete F2;
}

TEST_F(CloneInstruction, CallingConvention) {
  Type *ArgTy1[] = { Type::getInt32PtrTy(context) };
  FunctionType *FT1 =  FunctionType::get(Type::getVoidTy(context), ArgTy1, false);

  Function *F1 = Function::Create(FT1, Function::ExternalLinkage);
  F1->setCallingConv(CallingConv::Cold);
  BasicBlock *BB = BasicBlock::Create(context, "", F1);
  IRBuilder<> Builder(BB);
  Builder.CreateRetVoid();

  Function *F2 = Function::Create(FT1, Function::ExternalLinkage);

  SmallVector<ReturnInst*, 4> Returns;
  ValueToValueMapTy VMap;
  VMap[F1->arg_begin()] = F2->arg_begin();

  CloneFunctionInto(F2, F1, VMap, false, Returns);
  EXPECT_EQ(CallingConv::Cold, F2->getCallingConv());

  delete F1;
  delete F2;
}

class CloneFunc : public ::testing::Test {
protected:
  void SetUp() override {
    SetupModule();
    CreateOldFunc();
    CreateNewFunc();
    SetupFinder();
  }

  void TearDown() override { delete Finder; }

  void SetupModule() {
    M = new Module("", C);
  }

  void CreateOldFunc() {
    FunctionType* FuncType = FunctionType::get(Type::getVoidTy(C), false);
    OldFunc = Function::Create(FuncType, GlobalValue::PrivateLinkage, "f", M);
    CreateOldFunctionBodyAndDI();
  }

  void CreateOldFunctionBodyAndDI() {
    DIBuilder DBuilder(*M);
    IRBuilder<> IBuilder(C);

    // Function DI
    auto *File = DBuilder.createFile("filename.c", "/file/dir/");
    DITypeRefArray ParamTypes = DBuilder.getOrCreateTypeArray(None);
    DISubroutineType *FuncType =
        DBuilder.createSubroutineType(File, ParamTypes);
    auto *CU =
        DBuilder.createCompileUnit(dwarf::DW_LANG_C99, "filename.c",
                                   "/file/dir", "CloneFunc", false, "", 0);

    auto *Subprogram = DBuilder.createFunction(
        CU, "f", "f", File, 4, FuncType, true, true, 3, 0, false, OldFunc);

    // Function body
    BasicBlock* Entry = BasicBlock::Create(C, "", OldFunc);
    IBuilder.SetInsertPoint(Entry);
    DebugLoc Loc = DebugLoc::get(3, 2, Subprogram);
    IBuilder.SetCurrentDebugLocation(Loc);
    AllocaInst* Alloca = IBuilder.CreateAlloca(IntegerType::getInt32Ty(C));
    IBuilder.SetCurrentDebugLocation(DebugLoc::get(4, 2, Subprogram));
    Value* AllocaContent = IBuilder.getInt32(1);
    Instruction* Store = IBuilder.CreateStore(AllocaContent, Alloca);
    IBuilder.SetCurrentDebugLocation(DebugLoc::get(5, 2, Subprogram));
    Instruction* Terminator = IBuilder.CreateRetVoid();

    // Create a local variable around the alloca
    auto *IntType =
        DBuilder.createBasicType("int", 32, 0, dwarf::DW_ATE_signed);
    auto *E = DBuilder.createExpression();
    auto *Variable = DBuilder.createLocalVariable(
        dwarf::DW_TAG_auto_variable, Subprogram, "x", File, 5, IntType, true);
    auto *DL = DILocation::get(Subprogram->getContext(), 5, 0, Subprogram);
    DBuilder.insertDeclare(Alloca, Variable, E, DL, Store);
    DBuilder.insertDbgValueIntrinsic(AllocaContent, 0, Variable, E, DL,
                                     Terminator);
    // Finalize the debug info
    DBuilder.finalize();


    // Create another, empty, compile unit
    DIBuilder DBuilder2(*M);
    DBuilder2.createCompileUnit(dwarf::DW_LANG_C99,
        "extra.c", "/file/dir", "CloneFunc", false, "", 0);
    DBuilder2.finalize();
  }

  void CreateNewFunc() {
    ValueToValueMapTy VMap;
    NewFunc = CloneFunction(OldFunc, VMap, true, nullptr);
    M->getFunctionList().push_back(NewFunc);
  }

  void SetupFinder() {
    Finder = new DebugInfoFinder();
    Finder->processModule(*M);
  }

  LLVMContext C;
  Function* OldFunc;
  Function* NewFunc;
  Module* M;
  DebugInfoFinder* Finder;
};

// Test that a new, distinct function was created.
TEST_F(CloneFunc, NewFunctionCreated) {
  EXPECT_NE(OldFunc, NewFunc);
}

// Test that a new subprogram entry was added and is pointing to the new
// function, while the original subprogram still points to the old one.
TEST_F(CloneFunc, Subprogram) {
  EXPECT_FALSE(verifyModule(*M));

  unsigned SubprogramCount = Finder->subprogram_count();
  EXPECT_EQ(2U, SubprogramCount);

  auto Iter = Finder->subprograms().begin();
  auto *Sub1 = cast<DISubprogram>(*Iter);
  Iter++;
  auto *Sub2 = cast<DISubprogram>(*Iter);

  EXPECT_TRUE(
      (Sub1->getFunction() == OldFunc && Sub2->getFunction() == NewFunc) ||
      (Sub1->getFunction() == NewFunc && Sub2->getFunction() == OldFunc));
}

// Test that the new subprogram entry was not added to the CU which doesn't
// contain the old subprogram entry.
TEST_F(CloneFunc, SubprogramInRightCU) {
  EXPECT_FALSE(verifyModule(*M));

  EXPECT_EQ(2U, Finder->compile_unit_count());

  auto Iter = Finder->compile_units().begin();
  auto *CU1 = cast<DICompileUnit>(*Iter);
  Iter++;
  auto *CU2 = cast<DICompileUnit>(*Iter);
  EXPECT_TRUE(CU1->getSubprograms().size() == 0 ||
              CU2->getSubprograms().size() == 0);
}

// Test that instructions in the old function still belong to it in the
// metadata, while instruction in the new function belong to the new one.
TEST_F(CloneFunc, InstructionOwnership) {
  EXPECT_FALSE(verifyModule(*M));

  inst_iterator OldIter = inst_begin(OldFunc);
  inst_iterator OldEnd = inst_end(OldFunc);
  inst_iterator NewIter = inst_begin(NewFunc);
  inst_iterator NewEnd = inst_end(NewFunc);
  while (OldIter != OldEnd && NewIter != NewEnd) {
    Instruction& OldI = *OldIter;
    Instruction& NewI = *NewIter;
    EXPECT_NE(&OldI, &NewI);

    EXPECT_EQ(OldI.hasMetadata(), NewI.hasMetadata());
    if (OldI.hasMetadata()) {
      const DebugLoc& OldDL = OldI.getDebugLoc();
      const DebugLoc& NewDL = NewI.getDebugLoc();

      // Verify that the debug location data is the same
      EXPECT_EQ(OldDL.getLine(), NewDL.getLine());
      EXPECT_EQ(OldDL.getCol(), NewDL.getCol());

      // But that they belong to different functions
      auto *OldSubprogram = cast<DISubprogram>(OldDL.getScope());
      auto *NewSubprogram = cast<DISubprogram>(NewDL.getScope());
      EXPECT_EQ(OldFunc, OldSubprogram->getFunction());
      EXPECT_EQ(NewFunc, NewSubprogram->getFunction());
    }

    ++OldIter;
    ++NewIter;
  }
  EXPECT_EQ(OldEnd, OldIter);
  EXPECT_EQ(NewEnd, NewIter);
}

// Test that the arguments for debug intrinsics in the new function were
// properly cloned
TEST_F(CloneFunc, DebugIntrinsics) {
  EXPECT_FALSE(verifyModule(*M));

  inst_iterator OldIter = inst_begin(OldFunc);
  inst_iterator OldEnd = inst_end(OldFunc);
  inst_iterator NewIter = inst_begin(NewFunc);
  inst_iterator NewEnd = inst_end(NewFunc);
  while (OldIter != OldEnd && NewIter != NewEnd) {
    Instruction& OldI = *OldIter;
    Instruction& NewI = *NewIter;
    if (DbgDeclareInst* OldIntrin = dyn_cast<DbgDeclareInst>(&OldI)) {
      DbgDeclareInst* NewIntrin = dyn_cast<DbgDeclareInst>(&NewI);
      EXPECT_TRUE(NewIntrin);

      // Old address must belong to the old function
      EXPECT_EQ(OldFunc, cast<AllocaInst>(OldIntrin->getAddress())->
                         getParent()->getParent());
      // New address must belong to the new function
      EXPECT_EQ(NewFunc, cast<AllocaInst>(NewIntrin->getAddress())->
                         getParent()->getParent());

      // Old variable must belong to the old function
      EXPECT_EQ(OldFunc,
                cast<DISubprogram>(OldIntrin->getVariable()->getScope())
                    ->getFunction());
      // New variable must belong to the New function
      EXPECT_EQ(NewFunc,
                cast<DISubprogram>(NewIntrin->getVariable()->getScope())
                    ->getFunction());
    } else if (DbgValueInst* OldIntrin = dyn_cast<DbgValueInst>(&OldI)) {
      DbgValueInst* NewIntrin = dyn_cast<DbgValueInst>(&NewI);
      EXPECT_TRUE(NewIntrin);

      // Old variable must belong to the old function
      EXPECT_EQ(OldFunc,
                cast<DISubprogram>(OldIntrin->getVariable()->getScope())
                    ->getFunction());
      // New variable must belong to the New function
      EXPECT_EQ(NewFunc,
                cast<DISubprogram>(NewIntrin->getVariable()->getScope())
                    ->getFunction());
    }

    ++OldIter;
    ++NewIter;
  }
}

class CloneModule : public ::testing::Test {
protected:
  void SetUp() override {
    SetupModule();
    CreateOldModule();
    CreateNewModule();
  }

  void SetupModule() { OldM = new Module("", C); }

  void CreateOldModule() {
    IRBuilder<> IBuilder(C);

    auto *FuncType = FunctionType::get(Type::getVoidTy(C), false);
    auto *PersFn = Function::Create(FuncType, GlobalValue::ExternalLinkage,
                                    "persfn", OldM);
    auto *F =
        Function::Create(FuncType, GlobalValue::PrivateLinkage, "f", OldM);
    F->setPersonalityFn(PersFn);
    auto *Entry = BasicBlock::Create(C, "", F);
    IBuilder.SetInsertPoint(Entry);
    IBuilder.CreateRetVoid();
  }

  void CreateNewModule() { NewM = llvm::CloneModule(OldM); }

  LLVMContext C;
  Module *OldM;
  Module *NewM;
};

TEST_F(CloneModule, Verify) {
  EXPECT_FALSE(verifyModule(*NewM));
}

}
