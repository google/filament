//===- subzero/src/IceInstrumentation.cpp - ICE instrumentation framework -===//
//
//                        The Subzero Code Generator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Implements the Ice::Instrumentation class.
///
/// Subclasses can override particular instrumentation methods to specify how
/// the the target program should be instrumented.
///
//===----------------------------------------------------------------------===//

#include "IceInstrumentation.h"

#include "IceCfg.h"
#include "IceInst.h"
#include "IceTargetLowering.h"

namespace Ice {

// Iterate through the instructions in the given CFG and instrument each one.
// Also instrument the beginning of the function.
void Instrumentation::instrumentFunc(Cfg *Func) {
  assert(Func);
  assert(!Func->getNodes().empty());

  if (!isInstrumentable(Func))
    return;

  bool DidInstrumentEntry = false;
  LoweringContext Context;
  Context.init(Func->getNodes().front());
  for (CfgNode *Node : Func->getNodes()) {
    Context.init(Node);
    while (!Context.atEnd()) {
      if (!DidInstrumentEntry) {
        instrumentFuncStart(Context);
        DidInstrumentEntry = true;
      }
      instrumentInst(Context);
      // go to next undeleted instruction
      Context.advanceCur();
      Context.advanceNext();
    }
  }

  std::string FuncName = Func->getFunctionName().toStringOrEmpty();
  if (FuncName == "_start")
    instrumentStart(Func);

  finishFunc(Func);
}

void Instrumentation::instrumentInst(LoweringContext &Context) {
  assert(!Context.atEnd());
  Inst *Instr = iteratorToInst(Context.getCur());
  switch (Instr->getKind()) {
  case Inst::Alloca:
    instrumentAlloca(Context, llvm::cast<InstAlloca>(Instr));
    break;
  case Inst::Arithmetic:
    instrumentArithmetic(Context, llvm::cast<InstArithmetic>(Instr));
    break;
  case Inst::Br:
    instrumentBr(Context, llvm::cast<InstBr>(Instr));
    break;
  case Inst::Call:
    instrumentCall(Context, llvm::cast<InstCall>(Instr));
    break;
  case Inst::Cast:
    instrumentCast(Context, llvm::cast<InstCast>(Instr));
    break;
  case Inst::ExtractElement:
    instrumentExtractElement(Context, llvm::cast<InstExtractElement>(Instr));
    break;
  case Inst::Fcmp:
    instrumentFcmp(Context, llvm::cast<InstFcmp>(Instr));
    break;
  case Inst::Icmp:
    instrumentIcmp(Context, llvm::cast<InstIcmp>(Instr));
    break;
  case Inst::InsertElement:
    instrumentInsertElement(Context, llvm::cast<InstInsertElement>(Instr));
    break;
  case Inst::Intrinsic:
    instrumentIntrinsic(Context, llvm::cast<InstIntrinsic>(Instr));
    break;
  case Inst::Load:
    instrumentLoad(Context, llvm::cast<InstLoad>(Instr));
    break;
  case Inst::Phi:
    instrumentPhi(Context, llvm::cast<InstPhi>(Instr));
    break;
  case Inst::Ret:
    instrumentRet(Context, llvm::cast<InstRet>(Instr));
    break;
  case Inst::Select:
    instrumentSelect(Context, llvm::cast<InstSelect>(Instr));
    break;
  case Inst::Store:
    instrumentStore(Context, llvm::cast<InstStore>(Instr));
    break;
  case Inst::Switch:
    instrumentSwitch(Context, llvm::cast<InstSwitch>(Instr));
    break;
  case Inst::Unreachable:
    instrumentUnreachable(Context, llvm::cast<InstUnreachable>(Instr));
    break;
  default:
    // Only instrument high-level ICE instructions
    assert(false && "Instrumentation encountered an unexpected instruction");
    break;
  }
}

void Instrumentation::setHasSeenGlobals() {
  {
    std::unique_lock<std::mutex> _(GlobalsSeenMutex);
    HasSeenGlobals = true;
  }
  GlobalsSeenCV.notify_all();
}

LockedPtr<VariableDeclarationList> Instrumentation::getGlobals() {
  std::unique_lock<std::mutex> GlobalsLock(GlobalsSeenMutex);
  GlobalsSeenCV.wait(GlobalsLock, [this] { return HasSeenGlobals; });
  return Ctx->getGlobals();
}

} // end of namespace Ice
