///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilNonUniformResourceIndexInstrumentation.cpp                            //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Provides a pass to add instrumentation to determine missing usage of the  //
// NonUniformResourceIndex qualifier when dynamically indexing resources.    //
// Used by PIX.                                                              //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "PixPassHelpers.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DxilPIXPasses/DxilPIXPasses.h"
#include "dxc/DxilPIXPasses/DxilPIXVirtualRegisters.h"
#include "dxc/Support/Global.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;
using namespace hlsl;

class DxilNonUniformResourceIndexInstrumentation : public ModulePass {

public:
  static char ID; // Pass identification, replacement for typeid
  explicit DxilNonUniformResourceIndexInstrumentation() : ModulePass(ID) {}
  StringRef getPassName() const override {
    return "DXIL NonUniformResourceIndex Instrumentation";
  }
  bool runOnModule(Module &M) override;
};

bool DxilNonUniformResourceIndexInstrumentation::runOnModule(Module &M) {
  // This pass adds instrumentation for incorrect NonUniformResourceIndex usage

  DxilModule &DM = M.GetOrCreateDxilModule();
  LLVMContext &Ctx = M.getContext();
  OP *HlslOP = DM.GetOP();

  hlsl::DxilResource *PixUAVResource = nullptr;

  UndefValue *UndefArg = UndefValue::get(Type::getInt32Ty(Ctx));

  // Use WaveActiveAllEqual to check if a dynamic index is uniform
  Function *WaveActiveAllEqualFunc = HlslOP->GetOpFunc(
      DXIL::OpCode::WaveActiveAllEqual, Type::getInt32Ty(Ctx));
  Constant *WaveActiveAllEqualOpCode =
      HlslOP->GetI32Const((int32_t)DXIL::OpCode::WaveActiveAllEqual);

  // Atomic operation to use for writing to the result uav resource
  Function *AtomicOpFunc =
      HlslOP->GetOpFunc(OP::OpCode::AtomicBinOp, Type::getInt32Ty(Ctx));
  Constant *AtomicBinOpcode =
      HlslOP->GetU32Const((uint32_t)OP::OpCode::AtomicBinOp);
  Constant *AtomicOr = HlslOP->GetU32Const((uint32_t)DXIL::AtomicBinOpCode::Or);

  std::map<Function *, CallInst *> FunctionToUAVHandle;

  // This is the main pass that will iterate through all of the resources that
  // are dynamically indexed. If not already marked NonUniformResourceIndex,
  // then insert WaveActiveAllEqual to determine if the index is uniform
  // and finally write to a UAV resource with the result.

  PIXPassHelpers::ForEachDynamicallyIndexedResource(
      DM, [&](bool IsNonUniformIndex, Instruction *CreateHandle,
              Value *IndexOperand) {
        if (IsNonUniformIndex) {
          // The NonUniformResourceIndex qualifier was used, continue.
          return true;
        }

        if (!PixUAVResource) {
          PixUAVResource =
              PIXPassHelpers::CreateGlobalUAVResource(DM, 0, "PixUAVResource");
        }

        CallInst *PixUAVHandle = nullptr;
        Function *F = CreateHandle->getParent()->getParent();

        const auto FunctionToUAVHandleIter = FunctionToUAVHandle.lower_bound(F);

        if ((FunctionToUAVHandleIter != FunctionToUAVHandle.end()) &&
            (FunctionToUAVHandleIter->first == F)) {
          PixUAVHandle = FunctionToUAVHandleIter->second;
        } else {
          IRBuilder<> Builder(F->getEntryBlock().getFirstInsertionPt());

          PixUAVHandle = PIXPassHelpers::CreateHandleForResource(
              DM, Builder, PixUAVResource, "PixUAVHandle");

          FunctionToUAVHandle.insert(FunctionToUAVHandleIter,
                                     {F, PixUAVHandle});
        }

        IRBuilder<> Builder(CreateHandle);

        uint32_t InstructionNumber = 0;
        if (!pix_dxil::PixDxilInstNum::FromInst(CreateHandle,
                                                &InstructionNumber)) {
          DXASSERT_NOMSG(false);
        }

        // The output UAV is treated as a bit array where each bit corresponds
        // to an instruction number. This determines what byte offset to write
        // our result to based on the instruction number.
        const uint32_t InstructionNumByteOffset =
            (InstructionNumber / 32u) * sizeof(uint32_t);
        const uint32_t InstructionNumBitPosition = (InstructionNumber % 32u);
        const uint32_t InstructionNumBitMask = 1u << InstructionNumBitPosition;

        Constant *UAVByteOffsetArg =
            HlslOP->GetU32Const(InstructionNumByteOffset);

        CallInst *WaveActiveAllEqualCall = Builder.CreateCall(
            WaveActiveAllEqualFunc, {WaveActiveAllEqualOpCode, IndexOperand});

        // This takes the result of the WaveActiveAllEqual result and shifts
        // it into the same bit position as the instruction number, followed
        // by an xor to determine what to write to the UAV
        Value *IsWaveEqual =
            Builder.CreateZExt(WaveActiveAllEqualCall, Builder.getInt32Ty());
        Value *WaveEqualBitMask =
            Builder.CreateShl(IsWaveEqual, InstructionNumBitPosition);
        Value *FinalResult =
            Builder.CreateXor(WaveEqualBitMask, InstructionNumBitMask);

        // Generate instructions to bitwise OR a UAV value corresponding
        // to the instruction number and result of WaveActiveAllEqual.
        // If WaveActiveAllEqual was false, we write a 1, otherwise a 0.
        Builder.CreateCall(
            AtomicOpFunc,
            {
                AtomicBinOpcode,  // i32, ; opcode
                PixUAVHandle,     // %dx.types.Handle, ; resource handle
                AtomicOr,         // i32, ; binary operation code :
                                  // EXCHANGE, IADD, AND, OR, XOR
                                  // IMIN, IMAX, UMIN, UMAX
                UAVByteOffsetArg, // i32, ; coordinate c0: byte offset
                UndefArg,         // i32, ; coordinate c1 (unused)
                UndefArg,         // i32, ; coordinate c2 (unused)
                FinalResult       // i32);  value
            },
            "UAVInstructionNumberBitSet");
        return true;
      });

  const bool modified = (PixUAVResource != nullptr);

  if (modified) {
    DM.ReEmitDxilResources();

    if (OSOverride != nullptr) {
      formatted_raw_ostream FOS(*OSOverride);
      FOS << "\nFoundDynamicIndexingNoNuri\n";
    }
  }

  return modified;
}

char DxilNonUniformResourceIndexInstrumentation::ID = 0;

ModulePass *llvm::createDxilNonUniformResourceIndexInstrumentationPass() {
  return new DxilNonUniformResourceIndexInstrumentation();
}

INITIALIZE_PASS(DxilNonUniformResourceIndexInstrumentation,
                "hlsl-dxil-non-uniform-resource-index-instrumentation",
                "HLSL DXIL NonUniformResourceIndex instrumentation for PIX",
                false, false)
