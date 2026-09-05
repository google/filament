//===- DxilLoopDeletion.cpp - Dead Loop Deletion Pass -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file run LoopDeletion SimplifyCFG and DCE more than once to make sure
// all unused loop can be removed. Use kMaxIteration to avoid dead loop.
//
//===----------------------------------------------------------------------===//

#include "dxc/HLSL/DxilGenerationPass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"

using namespace llvm;

namespace {
class DxilLoopDeletion : public FunctionPass {
public:
  bool m_HLSLNoSink = false;
  static char ID; // Pass ID, replacement for typeid
  DxilLoopDeletion(bool NoSink = false)
      : FunctionPass(ID), m_HLSLNoSink(NoSink) {}

  bool runOnFunction(Function &F) override;

  void applyOptions(PassOptions O) override {
    GetPassOptionBool(O, "NoSink", &m_HLSLNoSink, /*defaultValue*/ false);
  }
  void dumpConfig(raw_ostream &OS) override {
    FunctionPass::dumpConfig(OS);
    OS << ",NoSink=" << m_HLSLNoSink;
  }
};
} // namespace

char DxilLoopDeletion::ID = 0;
INITIALIZE_PASS(DxilLoopDeletion, "dxil-loop-deletion",
                "Dxil Delete dead loops", false, false)

FunctionPass *llvm::createDxilLoopDeletionPass(bool NoSink) {
  return new DxilLoopDeletion(NoSink);
}

bool DxilLoopDeletion::runOnFunction(Function &F) {
  // Run loop simplify first to make sure loop invariant is moved so loop
  // deletion will not update the function if not delete.
  legacy::FunctionPassManager DeleteLoopPM(F.getParent());

  DeleteLoopPM.add(createLoopDeletionPass());
  bool bUpdated = false;

  legacy::FunctionPassManager SimplifyPM(F.getParent());
  SimplifyPM.add(createCFGSimplificationPass());
  SimplifyPM.add(createDeadCodeEliminationPass());
  SimplifyPM.add(createInstructionCombiningPass(/*HLSL No sink*/ m_HLSLNoSink));

  const unsigned kMaxIteration = 3;
  unsigned i = 0;
  while (i < kMaxIteration) {
    if (!DeleteLoopPM.run(F))
      break;

    SimplifyPM.run(F);
    i++;
    bUpdated = true;
  }

  return bUpdated;
}
