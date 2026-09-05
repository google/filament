///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// NormalizeDxil.cpp                                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Normalize DXIL transformation.                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "DxilConvPasses/NormalizeDxil.h"
#include "dxc/Support/Global.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"

#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilOperations.h"

#include <vector>

using namespace llvm;

//----------------------- Normalize Implementation ------------------------//

// Look for resource handles that were moved to the stack by reg2mem and
// move them back to registers.
//
// We make this change so the dxil will have an actual resource handle as
// the argument to a load/store resource instruction instead of being
// indirected through the stack.
class NormalizeResourceHandle {
public:
  bool Run(Function &F, DominatorTree &DT);

private:
  struct ResourceHandleCandidate {
    Instruction *Alloca;
    Instruction *CreateHandle;
  };
  Instruction *IsResourceHandleAllocaCandidate(BasicBlock *entryBlock,
                                               AllocaInst *allocaInst,
                                               DominatorTree &DT);
  void FindCandidates(BasicBlock &entry,
                      std::vector<ResourceHandleCandidate> &candidates,
                      DominatorTree &DT);
  void ReplaceResourceHandleUsage(
      const std::vector<ResourceHandleCandidate> &candidates,
      std::vector<Instruction *> &trash);
  void Cleanup(std::vector<Instruction *> &trash);
};

// Check to see if this is a valid resource handle location for replacement:
// 1. Only used in load/store.
// 2. Only stored to once.
// 3. Store value is create handle inst.
// 4. Create handle dominates all uses of alloca.
//
// The check is strict to limit the replacement candidates to those allocas that
// were inserted by mem2reg and make the replacement trivial.
Instruction *NormalizeResourceHandle::IsResourceHandleAllocaCandidate(
    BasicBlock *entryBlock, AllocaInst *allocaInst, DominatorTree &DT) {
  Instruction *createHandleInst = nullptr;
  Instruction *const NOT_A_CANDIDATE = nullptr;

  for (User *use : allocaInst->users()) {
    if (StoreInst *store = dyn_cast<StoreInst>(use)) {
      if (store->getPointerOperand() !=
          allocaInst) // In case it is used in gep expression.
        return NOT_A_CANDIDATE;

      Instruction *storedValue =
          dyn_cast<Instruction>(store->getValueOperand());
      if (!storedValue)
        return NOT_A_CANDIDATE;

      hlsl::DxilInst_CreateHandle createHandle(storedValue);
      if (!createHandle)
        return NOT_A_CANDIDATE;

      if (createHandleInst && createHandleInst != storedValue)
        return NOT_A_CANDIDATE;

      createHandleInst = storedValue;
    } else if (!(isa<LoadInst>(use))) {
      return NOT_A_CANDIDATE;
    }
  }

  for (Use &use : allocaInst->uses()) {
    if (!DT.dominates(createHandleInst, use))
      return NOT_A_CANDIDATE;
  }

  return createHandleInst;
}

void NormalizeResourceHandle::FindCandidates(
    BasicBlock &BBEntry, std::vector<ResourceHandleCandidate> &candidates,
    DominatorTree &DT) {
  DXASSERT_NOMSG(BBEntry.getTerminator());

  BasicBlock::iterator I = BBEntry.begin();
  while (isa<AllocaInst>(I)) {
    if (Instruction *createHandle = IsResourceHandleAllocaCandidate(
            &BBEntry, cast<AllocaInst>(I), DT)) {
      candidates.push_back({I, createHandle});
    }
    ++I;
  }
}

void NormalizeResourceHandle::ReplaceResourceHandleUsage(
    const std::vector<ResourceHandleCandidate> &candidates,
    std::vector<Instruction *> &trash) {
  for (const ResourceHandleCandidate &candidate : candidates) {
    for (User *use : candidate.Alloca->users()) {
      if (LoadInst *load = dyn_cast<LoadInst>(use)) {
        load->replaceAllUsesWith(candidate.CreateHandle);
        trash.push_back(load);
      } else if (StoreInst *store = dyn_cast<StoreInst>(use)) {
        trash.push_back(store);
      } else {
        DXASSERT(false, "should only have load and store insts");
      }
    }

    trash.push_back(candidate.Alloca);
  }
}

void NormalizeResourceHandle::Cleanup(std::vector<Instruction *> &trash) {
  for (Instruction *inst : trash) {
    inst->eraseFromParent();
  }

  trash.clear();
}

bool NormalizeResourceHandle::Run(Function &function, DominatorTree &DT) {
  std::vector<ResourceHandleCandidate> candidates;
  std::vector<Instruction *> trash;

  FindCandidates(function.getEntryBlock(), candidates, DT);
  ReplaceResourceHandleUsage(candidates, trash);
  Cleanup(trash);

  return candidates.size() > 0;
}

bool NormalizeDxil::Run() {
  return NormalizeResourceHandle().Run(m_function, m_dominatorTree);
}

//----------------------- Pass Implementation ------------------------//
char NormalizeDxilPass::ID = 0;
INITIALIZE_PASS_BEGIN(NormalizeDxilPass, "normalizedxil", "Normalize dxil pass",
                      false, false)
INITIALIZE_PASS_END(NormalizeDxilPass, "normalizedxil", "Normalize dxil pass",
                    false, false)

FunctionPass *llvm::createNormalizeDxilPass() {
  return new NormalizeDxilPass();
}

bool NormalizeDxilPass::runOnFunction(Function &F) {
  DominatorTree &DT = getAnalysis<DominatorTreeWrapperPass>().getDomTree();
  NormalizeDxil normalizer(F, DT);
  return normalizer.Run();
}

void NormalizeDxilPass::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<DominatorTreeWrapperPass>();
}
