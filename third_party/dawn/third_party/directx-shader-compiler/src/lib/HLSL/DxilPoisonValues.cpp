///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilPoisonValues.cpp                                                      //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Allows insertion of poisoned values with error messages that get          //
// cleaned up late in the compiler.                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/HLSL/DxilPoisonValues.h"

#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

constexpr const char kPoisonPrefix[] = "dx.poison.";

namespace hlsl {
Value *CreatePoisonValue(Type *ty, const Twine &errMsg, DebugLoc DL,
                         Instruction *InsertPt) {
  std::string functionName;
  {
    llvm::raw_string_ostream os(functionName);
    os << kPoisonPrefix;
    os << *ty;
    os.flush();
  }

  Module &M = *InsertPt->getModule();

  LLVMContext &C = M.getContext();
  Type *argTypes[] = {Type::getMetadataTy(C)};
  FunctionType *ft = FunctionType::get(ty, argTypes, false);
  Constant *f = M.getOrInsertFunction(functionName, ft);

  std::string errMsgStr = errMsg.str();
  Value *args[] = {MetadataAsValue::get(C, MDString::get(C, errMsgStr))};
  CallInst *ret = CallInst::Create(f, ArrayRef<Value *>(args), "err", InsertPt);
  ret->setDebugLoc(DL);
  return ret;
}

bool FinalizePoisonValues(Module &M) {
  bool changed = false;
  LLVMContext &Ctx = M.getContext();
  for (auto it = M.begin(); it != M.end();) {
    Function *F = &*(it++);
    if (F->getName().startswith(kPoisonPrefix)) {
      for (auto it = F->user_begin(); it != F->user_end();) {
        User *U = *(it++);
        CallInst *call = cast<CallInst>(U);
        MDString *errMsgMD = cast<MDString>(
            cast<MetadataAsValue>(call->getArgOperand(0))->getMetadata());
        StringRef errMsg = errMsgMD->getString();

        Ctx.diagnose(
            DiagnosticInfoDxil(F, call->getDebugLoc(), errMsg, DS_Error));
        if (!call->getType()->isVoidTy())
          call->replaceAllUsesWith(UndefValue::get(call->getType()));
        call->eraseFromParent();
      }
      F->eraseFromParent();
      changed = true;
    }
  }
  return changed;
}
} // namespace hlsl
