///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilScalarizeVectorLoadStores.cpp                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Lowers native vector load stores to potentially multiple scalar calls.    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/HLSL/DxilGenerationPass.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace hlsl;

static void scalarizeVectorLoad(hlsl::OP *HlslOP, const DataLayout &DL,
                                CallInst *CI);
static void scalarizeVectorStore(hlsl::OP *HlslOP, const DataLayout &DL,
                                 CallInst *CI);

class DxilScalarizeVectorLoadStores : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  explicit DxilScalarizeVectorLoadStores() : ModulePass(ID) {}

  StringRef getPassName() const override {
    return "DXIL scalarize vector load/stores";
  }

  bool runOnModule(Module &M) override {
    DxilModule &DM = M.GetOrCreateDxilModule();
    // Shader Model 6.9 allows native vectors and doesn't need this pass.
    if (DM.GetShaderModel()->IsSM69Plus())
      return false;

    bool Changed = false;

    hlsl::OP *HlslOP = DM.GetOP();
    for (auto FIt : HlslOP->GetOpFuncList(DXIL::OpCode::RawBufferVectorLoad)) {
      Function *Func = FIt.second;
      if (!Func)
        continue;
      for (auto U = Func->user_begin(), UE = Func->user_end(); U != UE;) {
        CallInst *CI = cast<CallInst>(*(U++));
        scalarizeVectorLoad(HlslOP, M.getDataLayout(), CI);
        Changed = true;
      }
    }
    for (auto FIt : HlslOP->GetOpFuncList(DXIL::OpCode::RawBufferVectorStore)) {
      Function *Func = FIt.second;
      if (!Func)
        continue;
      for (auto U = Func->user_begin(), UE = Func->user_end(); U != UE;) {
        CallInst *CI = cast<CallInst>(*(U++));
        scalarizeVectorStore(HlslOP, M.getDataLayout(), CI);
        Changed = true;
      }
    }
    return Changed;
  }
};

static unsigned GetRawBufferMask(unsigned NumComponents) {
  switch (NumComponents) {
  case 0:
    return 0;
  case 1:
    return DXIL::kCompMask_X;
  case 2:
    return DXIL::kCompMask_X | DXIL::kCompMask_Y;
  case 3:
    return DXIL::kCompMask_X | DXIL::kCompMask_Y | DXIL::kCompMask_Z;
  case 4:
  default:
    return DXIL::kCompMask_All;
  }
  return DXIL::kCompMask_All;
}

static void scalarizeVectorLoad(hlsl::OP *HlslOP, const DataLayout &DL,
                                CallInst *CI) {
  IRBuilder<> Builder(CI);
  // Collect the information required to break this into scalar ops from args.
  DxilInst_RawBufferVectorLoad VecLd(CI);
  OP::OpCode OpCode = OP::OpCode::RawBufferLoad;
  llvm::Constant *OpArg = Builder.getInt32((unsigned)OpCode);
  SmallVector<Value *, 10> Args;
  Args.emplace_back(OpArg);                     // opcode @0.
  Args.emplace_back(VecLd.get_buf());           // Resource handle @1.
  Args.emplace_back(VecLd.get_index());         // Index @2.
  Args.emplace_back(VecLd.get_elementOffset()); // Offset @3.
  Args.emplace_back(nullptr);                   // Mask to be set later @4.
  Args.emplace_back(VecLd.get_alignment());     // Alignment @5.

  // Set offset to increment depending on whether the real offset is defined.
  unsigned OffsetIdx;
  if (isa<UndefValue>(VecLd.get_elementOffset()))
    // Byte Address Buffers can't use offset, so use index.
    OffsetIdx = DXIL::OperandIndex::kRawBufferLoadIndexOpIdx;
  else
    OffsetIdx = DXIL::OperandIndex::kRawBufferLoadElementOffsetOpIdx;

  StructType *ResRetTy = cast<StructType>(CI->getType());
  Type *Ty = ResRetTy->getElementType(0);
  unsigned NumComponents = Ty->getVectorNumElements();
  Type *EltTy = Ty->getScalarType();
  unsigned EltSize = DL.getTypeAllocSize(EltTy);

  const unsigned MaxElemCount = 4;
  SmallVector<Value *, 4> Elts(NumComponents);
  Value *Ld = nullptr;
  for (unsigned EIx = 0; EIx < NumComponents;) {
    // Load 4 elements or however many less than 4 are left to load.
    unsigned ChunkSize = std::min(NumComponents - EIx, MaxElemCount);
    Args[DXIL::OperandIndex::kRawBufferLoadMaskOpIdx] =
        HlslOP->GetI8Const(GetRawBufferMask(ChunkSize));
    // If we've loaded a chunk already, update offset to next chunk.
    if (EIx > 0)
      Args[OffsetIdx] =
          Builder.CreateAdd(Args[OffsetIdx], HlslOP->GetU32Const(4 * EltSize));
    Function *F = HlslOP->GetOpFunc(OpCode, EltTy);
    Ld = Builder.CreateCall(F, Args, OP::GetOpCodeName(OpCode));
    for (unsigned ChIx = 0; ChIx < ChunkSize; ChIx++, EIx++)
      Elts[EIx] = Builder.CreateExtractValue(Ld, ChIx);
  }

  Value *RetValNew = UndefValue::get(VectorType::get(EltTy, NumComponents));
  for (unsigned ElIx = 0; ElIx < NumComponents; ElIx++)
    RetValNew = Builder.CreateInsertElement(RetValNew, Elts[ElIx], ElIx);

  // Replace users of the vector extracted from the vector load resret.
  Value *Status = nullptr;
  for (auto CU = CI->user_begin(), CE = CI->user_end(); CU != CE;) {
    auto EV = cast<ExtractValueInst>(*(CU++));
    unsigned Ix = EV->getIndices()[0];
    if (Ix == 0) {
      // Handle value uses.
      EV->replaceAllUsesWith(RetValNew);
    } else if (Ix == 1) {
      // Handle status uses.
      if (!Status)
        Status = Builder.CreateExtractValue(Ld, DXIL::kResRetStatusIndex);
      EV->replaceAllUsesWith(Status);
    }
    EV->eraseFromParent();
  }
  CI->eraseFromParent();
}

static void scalarizeVectorStore(hlsl::OP *HlslOP, const DataLayout &DL,
                                 CallInst *CI) {
  IRBuilder<> Builder(CI);
  // Collect the information required to break this into scalar ops from args.
  DxilInst_RawBufferVectorStore VecSt(CI);
  OP::OpCode OpCode = OP::OpCode::RawBufferStore;
  llvm::Constant *OpArg = Builder.getInt32((unsigned)OpCode);
  SmallVector<Value *, 10> Args;
  Args.emplace_back(OpArg);                     // opcode @0.
  Args.emplace_back(VecSt.get_uav());           // Resource handle @1.
  Args.emplace_back(VecSt.get_index());         // Index @2.
  Args.emplace_back(VecSt.get_elementOffset()); // Offset @3.
  Args.emplace_back(nullptr);                   // Val0 to be set later @4.
  Args.emplace_back(nullptr);                   // Val1 to be set later @5.
  Args.emplace_back(nullptr);                   // Val2 to be set later @6.
  Args.emplace_back(nullptr);                   // Val3 to be set later @7.
  Args.emplace_back(nullptr);                   // Mask to be set later @8.
  Args.emplace_back(VecSt.get_alignment());     // Alignment @9.

  // Set offset to increment depending on whether the real offset is defined.
  unsigned OffsetIdx;
  if (isa<UndefValue>(VecSt.get_elementOffset()))
    // Byte Address Buffers can't use offset, so use index.
    OffsetIdx = DXIL::OperandIndex::kRawBufferLoadIndexOpIdx;
  else
    OffsetIdx = DXIL::OperandIndex::kRawBufferLoadElementOffsetOpIdx;

  Value *VecVal = VecSt.get_value0();

  const unsigned MaxElemCount = 4;
  Type *Ty = VecVal->getType();
  const unsigned NumComponents = Ty->getVectorNumElements();
  Type *EltTy = Ty->getScalarType();
  Value *UndefVal = UndefValue::get(EltTy);
  unsigned EltSize = DL.getTypeAllocSize(EltTy);
  Function *F = HlslOP->GetOpFunc(OpCode, EltTy);
  for (unsigned EIx = 0; EIx < NumComponents;) {
    // Store 4 elements or however many less than 4 are left to store.
    unsigned ChunkSize = std::min(NumComponents - EIx, MaxElemCount);
    // For second and subsequent store calls, increment the resource-appropriate
    // index or offset parameter.
    if (EIx > 0)
      Args[OffsetIdx] =
          Builder.CreateAdd(Args[OffsetIdx], HlslOP->GetU32Const(4 * EltSize));
    // Populate all value arguments either with the vector or undefs.
    uint8_t Mask = 0;
    unsigned ChIx = 0;
    for (; ChIx < ChunkSize; ChIx++, EIx++) {
      Args[DXIL::OperandIndex::kRawBufferStoreVal0OpIdx + ChIx] =
          Builder.CreateExtractElement(VecVal, EIx);
      Mask |= (1 << ChIx);
    }
    for (; ChIx < MaxElemCount; ChIx++)
      Args[DXIL::OperandIndex::kRawBufferStoreVal0OpIdx + ChIx] = UndefVal;

    Args[DXIL::OperandIndex::kRawBufferStoreMaskOpIdx] =
        HlslOP->GetU8Const(Mask);
    Builder.CreateCall(F, Args);
  }
  CI->eraseFromParent();
}

char DxilScalarizeVectorLoadStores::ID = 0;

ModulePass *llvm::createDxilScalarizeVectorLoadStoresPass() {
  return new DxilScalarizeVectorLoadStores();
}

INITIALIZE_PASS(DxilScalarizeVectorLoadStores,
                "hlsl-dxil-scalarize-vector-load-stores",
                "DXIL scalarize vector load/stores", false, false)
