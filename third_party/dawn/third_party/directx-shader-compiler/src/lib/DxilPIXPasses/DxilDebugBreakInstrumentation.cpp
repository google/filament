///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilDebugBreakInstrumentation.cpp                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides a pass to instrument DebugBreak() calls for PIX. Each           //
// DebugBreak call is replaced with a UAV bit-write so PIX can detect       //
// which DebugBreak locations were hit without halting the GPU.              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "PixPassHelpers.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DxilPIXPasses/DxilPIXPasses.h"
#include "dxc/DxilPIXPasses/DxilPIXVirtualRegisters.h"
#include "dxc/Support/Global.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;
using namespace hlsl;

class DxilDebugBreakInstrumentation : public ModulePass {

public:
  static char ID; // Pass identification, replacement for typeid
  explicit DxilDebugBreakInstrumentation() : ModulePass(ID) {}
  StringRef getPassName() const override {
    return "DXIL DebugBreak Instrumentation";
  }
  bool runOnModule(Module &M) override;
};

bool DxilDebugBreakInstrumentation::runOnModule(Module &M) {
  DxilModule &DM = M.GetOrCreateDxilModule();
  LLVMContext &Ctx = M.getContext();
  OP *HlslOP = DM.GetOP();

  hlsl::DxilResource *PixUAVResource = nullptr;

  UndefValue *UndefArg = UndefValue::get(Type::getInt32Ty(Ctx));

  // Atomic operation to use for writing to the result UAV resource
  Function *AtomicOpFunc =
      HlslOP->GetOpFunc(OP::OpCode::AtomicBinOp, Type::getInt32Ty(Ctx));
  Constant *AtomicBinOpcode =
      HlslOP->GetU32Const((uint32_t)OP::OpCode::AtomicBinOp);
  Constant *AtomicOr = HlslOP->GetU32Const((uint32_t)DXIL::AtomicBinOpCode::Or);

  std::map<Function *, CallInst *> FunctionToUAVHandle;

  // Collect all DebugBreak calls first, then modify.
  // This avoids invalidating iterators during modification.
  std::vector<CallInst *> DebugBreakCalls;

  Function *DebugBreakFunc =
      HlslOP->GetOpFunc(OP::OpCode::DebugBreak, Type::getVoidTy(Ctx));
  for (const Use &U : DebugBreakFunc->uses()) {
    DebugBreakCalls.push_back(cast<CallInst>(U.getUser()));
  }

  for (CallInst *CI : DebugBreakCalls) {
    if (!PixUAVResource)
      PixUAVResource =
          PIXPassHelpers::CreateGlobalUAVResource(DM, 0, "PixUAVResource");

    Function *F = CI->getParent()->getParent();

    CallInst *PixUAVHandle = nullptr;
    const auto FunctionToUAVHandleIter = FunctionToUAVHandle.lower_bound(F);

    if ((FunctionToUAVHandleIter != FunctionToUAVHandle.end()) &&
        (FunctionToUAVHandleIter->first == F)) {
      PixUAVHandle = FunctionToUAVHandleIter->second;
    } else {
      IRBuilder<> Builder(F->getEntryBlock().getFirstInsertionPt());

      PixUAVHandle = PIXPassHelpers::CreateHandleForResource(
          DM, Builder, PixUAVResource, "PixUAVHandle");

      FunctionToUAVHandle.insert(FunctionToUAVHandleIter, {F, PixUAVHandle});
    }

    IRBuilder<> Builder(CI);

    uint32_t InstructionNumber = 0;
    if (!pix_dxil::PixDxilInstNum::FromInst(CI, &InstructionNumber)) {
      DXASSERT(false, "Failed to extract PIX instruction number metadata from "
                      "DebugBreak call");
    }

    // The output UAV is treated as a bit array where each bit corresponds
    // to an instruction number.
    const uint32_t InstructionNumByteOffset =
        (InstructionNumber / 32u) * sizeof(uint32_t);
    const uint32_t InstructionNumBitPosition = (InstructionNumber % 32u);
    const uint32_t InstructionNumBitMask = 1u << InstructionNumBitPosition;

    Constant *UAVByteOffsetArg = HlslOP->GetU32Const(InstructionNumByteOffset);
    Constant *BitMaskArg = HlslOP->GetU32Const(InstructionNumBitMask);

    // Write a 1 bit at the position corresponding to this DebugBreak's
    // instruction number, indicating it was hit.
    Builder.CreateCall(
        AtomicOpFunc,
        {
            AtomicBinOpcode,  // i32, ; opcode
            PixUAVHandle,     // %dx.types.Handle, ; resource handle
            AtomicOr,         // i32, ; binary operation code
            UAVByteOffsetArg, // i32, ; coordinate c0: byte offset
            UndefArg,         // i32, ; coordinate c1 (unused)
            UndefArg,         // i32, ; coordinate c2 (unused)
            BitMaskArg        // i32);  value
        },
        "DebugBreakBitSet");

    // Remove the original DebugBreak call to prevent GPU halt
    CI->eraseFromParent();
  }

  // Clean up the now-unused declaration. Not strictly required for
  // correctness, but keeps the module free of dead references.
  if (DebugBreakFunc->use_empty())
    DebugBreakFunc->eraseFromParent();

  const bool modified = (PixUAVResource != nullptr);

  if (modified) {
    DM.ReEmitDxilResources();

    if (OSOverride != nullptr) {
      formatted_raw_ostream FOS(*OSOverride);
      FOS << "\nFoundDebugBreak\n";
    }
  }

  return modified;
}

char DxilDebugBreakInstrumentation::ID = 0;

ModulePass *llvm::createDxilDebugBreakInstrumentationPass() {
  return new DxilDebugBreakInstrumentation();
}

INITIALIZE_PASS(DxilDebugBreakInstrumentation,
                "hlsl-dxil-debugbreak-instrumentation",
                "HLSL DXIL DebugBreak instrumentation for PIX", false, false)
