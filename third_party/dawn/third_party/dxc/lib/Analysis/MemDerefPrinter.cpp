//===- MemDerefPrinter.cpp - Printer for isDereferenceablePointer ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm/Analysis/Passes.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

namespace {
  struct MemDerefPrinter : public FunctionPass {
    SmallVector<Value *, 4> Vec;

    static char ID; // Pass identification, replacement for typeid
    MemDerefPrinter() : FunctionPass(ID) {
      initializeMemDerefPrinterPass(*PassRegistry::getPassRegistry());
    }
    void getAnalysisUsage(AnalysisUsage &AU) const override {
      AU.setPreservesAll();
    }
    bool runOnFunction(Function &F) override;
    void print(raw_ostream &OS, const Module * = nullptr) const override;
    void releaseMemory() override {
      Vec.clear();
    }
  };
}

char MemDerefPrinter::ID = 0;
INITIALIZE_PASS_BEGIN(MemDerefPrinter, "print-memderefs",
                      "Memory Dereferenciblity of pointers in function", false, true)
INITIALIZE_PASS_END(MemDerefPrinter, "print-memderefs",
                    "Memory Dereferenciblity of pointers in function", false, true)

FunctionPass *llvm::createMemDerefPrinter() {
  return new MemDerefPrinter();
}

bool MemDerefPrinter::runOnFunction(Function &F) {
  const DataLayout &DL = F.getParent()->getDataLayout();
  for (auto &I: inst_range(F)) {
    if (LoadInst *LI = dyn_cast<LoadInst>(&I)) {
      Value *PO = LI->getPointerOperand();
      if (isDereferenceablePointer(PO, DL))
        Vec.push_back(PO);
    }
  }
  return false;
}

void MemDerefPrinter::print(raw_ostream &OS, const Module *M) const {
  OS << "The following are dereferenceable:\n";
  for (auto &V: Vec) {
    V->print(OS);
    OS << "\n\n";
  }
}
