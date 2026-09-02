///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilScalarizeVectorIntrinsics.cpp                                         //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Lowers native vector load stores to potentially multiple scalar calls.    //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/HLSL/DxilGenerationPass.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Pass.h"

using namespace llvm;
using namespace hlsl;

static bool scalarizeVectorLoad(hlsl::OP *HlslOP, const DataLayout &DL,
                                CallInst *CI);
static bool scalarizeVectorStore(hlsl::OP *HlslOP, const DataLayout &DL,
                                 CallInst *CI);
static bool scalarizeVectorIntrinsic(hlsl::OP *HlslOP, CallInst *CI);
static bool scalarizeVectorReduce(hlsl::OP *HlslOP, CallInst *CI);
static bool scalarizeVectorDot(hlsl::OP *HlslOP, CallInst *CI);
static bool scalarizeVectorWaveMatch(hlsl::OP *HlslOP, CallInst *CI);

class DxilScalarizeVectorIntrinsics : public ModulePass {
public:
  static char ID; // Pass identification, replacement for typeid
  explicit DxilScalarizeVectorIntrinsics() : ModulePass(ID) {}

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

    // Iterate and scalarize native vector loads, stores, and other intrinsics.
    for (auto F = M.functions().begin(); F != M.functions().end();) {
      Function *Func = &*(F++);
      DXIL::OpCodeClass OpClass;
      if (!HlslOP->GetOpCodeClass(Func, OpClass))
        continue;

      const bool CouldRewrite =
          (Func->getReturnType()->isVectorTy() ||
           OpClass == DXIL::OpCodeClass::RawBufferVectorLoad ||
           OpClass == DXIL::OpCodeClass::RawBufferVectorStore ||
           OpClass == DXIL::OpCodeClass::VectorReduce ||
           OpClass == DXIL::OpCodeClass::Dot ||
           OpClass == DXIL::OpCodeClass::WaveMatch);
      if (!CouldRewrite)
        continue;

      for (auto U = Func->user_begin(), UE = Func->user_end(); U != UE;) {
        CallInst *CI = cast<CallInst>(*(U++));

        // Handle DXIL operations with complex signatures separately
        switch (OpClass) {
        case DXIL::OpCodeClass::RawBufferVectorLoad:
          Changed |= scalarizeVectorLoad(HlslOP, M.getDataLayout(), CI);
          continue;
        case DXIL::OpCodeClass::RawBufferVectorStore:
          Changed |= scalarizeVectorStore(HlslOP, M.getDataLayout(), CI);
          continue;
        case DXIL::OpCodeClass::VectorReduce:
          Changed |= scalarizeVectorReduce(HlslOP, CI);
          continue;
        case DXIL::OpCodeClass::Dot:
          Changed |= scalarizeVectorDot(HlslOP, CI);
          continue;
        case DXIL::OpCodeClass::WaveMatch:
          Changed |= scalarizeVectorWaveMatch(HlslOP, CI);
          continue;
        default:
          break;
        }

        // Handle DXIL Ops with vector return matching the vector params
        if (Func->getReturnType()->isVectorTy())
          Changed |= scalarizeVectorIntrinsic(HlslOP, CI);
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

static bool scalarizeVectorLoad(hlsl::OP *HlslOP, const DataLayout &DL,
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
  return true;
}

static bool scalarizeVectorStore(hlsl::OP *HlslOP, const DataLayout &DL,
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
  return true;
}

static bool scalarizeVectorReduce(hlsl::OP *HlslOP, CallInst *CI) {
  IRBuilder<> Builder(CI);

  OP::OpCode ReduceOp = OP::getOpCode(CI);

  Value *VecArg = CI->getArgOperand(1);
  Type *VecTy = VecArg->getType();

  Value *Result = Builder.CreateExtractElement(VecArg, (uint64_t)0);
  for (unsigned I = 1; I < VecTy->getVectorNumElements(); I++) {
    Value *Elt = Builder.CreateExtractElement(VecArg, I);

    switch (ReduceOp) {
    case OP::OpCode::VectorReduceAnd:
      Result = Builder.CreateAnd(Result, Elt);
      break;
    case OP::OpCode::VectorReduceOr:
      Result = Builder.CreateOr(Result, Elt);
      break;
    default:
      assert(false && "Unexpected VectorReduce OpCode");
    }
  }

  CI->replaceAllUsesWith(Result);
  CI->eraseFromParent();
  return true;
}

static bool scalarizeVectorWaveMatch(hlsl::OP *HlslOP, CallInst *CI) {
  IRBuilder<> Builder(CI);
  OP::OpCode Opcode = OP::getOpCode(CI);
  Value *VecArg = CI->getArgOperand(1);
  Type *VecTy = VecArg->getType();

  // Non-vector parameters don't need to be scalarized
  if (!VecTy->isVectorTy())
    return false;

  const uint64_t VecSize = VecTy->getVectorNumElements();
  Type *ScalarTy = VecTy->getScalarType();
  Function *Func = HlslOP->GetOpFunc(Opcode, ScalarTy);
  SmallVector<Value *, 2> Args(CI->getNumArgOperands());
  Args[0] = CI->getArgOperand(0); // Copy opcode over.

  SmallVector<Value *, 4> Scalars;
  for (uint64_t I = 0; I != VecSize; ++I) {
    Scalars.push_back(Builder.CreateExtractElement(VecArg, I));
  }

  SmallVector<Value *, 4> ResultVecs;
  for (uint64_t I = 0; I != VecSize; ++I) {
    Args[1] = Scalars[I];
    ResultVecs.push_back(Builder.CreateCall(Func, Args));
  }

  Value *Ret = ResultVecs[0];
  for (uint64_t I = 1; I != VecSize; ++I) {
    Value *Next = ResultVecs[I];

    // Generate bitwise AND of the components
    Value *LHS0 = Builder.CreateExtractValue(Ret, 0);
    Value *LHS1 = Builder.CreateExtractValue(Ret, 1);
    Value *LHS2 = Builder.CreateExtractValue(Ret, 2);
    Value *LHS3 = Builder.CreateExtractValue(Ret, 3);
    Value *RHS0 = Builder.CreateExtractValue(Next, 0);
    Value *RHS1 = Builder.CreateExtractValue(Next, 1);
    Value *RHS2 = Builder.CreateExtractValue(Next, 2);
    Value *RHS3 = Builder.CreateExtractValue(Next, 3);
    Value *And0 = Builder.CreateAnd(LHS0, RHS0);
    Value *And1 = Builder.CreateAnd(LHS1, RHS1);
    Value *And2 = Builder.CreateAnd(LHS2, RHS2);
    Value *And3 = Builder.CreateAnd(LHS3, RHS3);
    Ret = Builder.CreateInsertValue(Ret, And0, 0);
    Ret = Builder.CreateInsertValue(Ret, And1, 1);
    Ret = Builder.CreateInsertValue(Ret, And2, 2);
    Ret = Builder.CreateInsertValue(Ret, And3, 3);
  }

  CI->replaceAllUsesWith(Ret);
  CI->eraseFromParent();
  return true;
}

// Scalarize vectorized dot product
static bool scalarizeVectorDot(hlsl::OP *HlslOP, CallInst *CI) {
  IRBuilder<> Builder(CI);

  Value *AVecArg = CI->getArgOperand(1);
  Value *BVecArg = CI->getArgOperand(2);
  VectorType *VecTy = cast<VectorType>(AVecArg->getType());
  Type *ScalarTy = VecTy->getScalarType();
  const unsigned VecSize = VecTy->getNumElements();

  // The only valid opcode is FDot which only has floating point overload.
  // If we hit this assert then this functions lowering needs to be updated
  assert(ScalarTy->isFloatingPointTy() && "Unexpected scalar type");

  SmallVector<Value *, 4> AElts(VecSize);
  SmallVector<Value *, 4> BElts(VecSize);

  for (unsigned EltIdx = 0; EltIdx < VecSize; EltIdx++) {
    AElts[EltIdx] = Builder.CreateExtractElement(AVecArg, EltIdx);
    BElts[EltIdx] = Builder.CreateExtractElement(BVecArg, EltIdx);
  }

  DXIL::OpCode DotOp = DXIL::OpCode::Dot4;
  switch (VecSize) {
  // Calling dot on a vec1 is not typical but also not impossible
  // DXIL doesn't have a native Dot1 opcode but thats the same as a
  // single FMul. HLOperation lower is expected to do the conversion
  // so we assert here in case that ever changes.
  case 1:
    assert(false && "vector dot shouldn't appear for vec1");
    break;
  case 2:
    DotOp = DXIL::OpCode::Dot2;
    break;
  case 3:
    DotOp = DXIL::OpCode::Dot3;
    break;
  case 4:
    DotOp = DXIL::OpCode::Dot4;
    break;
  default:
    assert(false &&
           "Vectors larger than 4 components are not supported in SM6.8");
    break;
  }

  SmallVector<Value *, 9> Args(VecSize * 2 + 1);
  Args[0] = Builder.getInt32((unsigned)DotOp);

  for (unsigned EltIdx = 0; EltIdx < VecSize; EltIdx++) {
    Args[EltIdx + 1] = AElts[EltIdx];
    Args[EltIdx + 1 + VecSize] = BElts[EltIdx];
  }

  Function *Func = HlslOP->GetOpFunc(DotOp, ScalarTy);
  Value *Dot = Builder.CreateCall(Func, Args, CI->getName());
  CI->replaceAllUsesWith(Dot);
  return true;
}

// Scalarize native vector operation represented by `CI`, generating
// scalar calls for each element of the its vector parameters.
// Use `HlslOP` to retrieve the associated scalar op function.
static bool scalarizeVectorIntrinsic(hlsl::OP *HlslOP, CallInst *CI) {

  IRBuilder<> Builder(CI);
  VectorType *VT = cast<VectorType>(CI->getType());
  unsigned VecSize = VT->getNumElements();
  unsigned ArgNum = CI->getNumArgOperands();
  OP::OpCode Opcode = OP::getOpCode(CI);
  Type *Ty = OP::GetOverloadType(Opcode, CI->getCalledFunction());
  Function *Func = HlslOP->GetOpFunc(Opcode, Ty->getScalarType());
  SmallVector<Value *, 4> Args(ArgNum);
  Args[0] = CI->getArgOperand(0); // Copy opcode over.

  // For each element in the vector, generate a new call instruction.
  // Insert results into a result vector.
  Value *RetVal = UndefValue::get(CI->getType());
  for (unsigned ElIx = 0; ElIx < VecSize; ElIx++) {
    // Replace each vector argument with the result of an extraction.
    // Skip known opcode arg as it can't be a vector.
    for (unsigned ArgIx = 1; ArgIx < ArgNum; ArgIx++) {
      Value *Arg = CI->getArgOperand(ArgIx);
      if (Arg->getType()->isVectorTy())
        Args[ArgIx] = Builder.CreateExtractElement(Arg, ElIx);
      else
        Args[ArgIx] = Arg;
    }
    Value *ElCI = Builder.CreateCall(Func, Args, CI->getName());
    RetVal = Builder.CreateInsertElement(RetVal, ElCI, ElIx);
  }

  CI->replaceAllUsesWith(RetVal);
  CI->eraseFromParent();
  return true;
}

char DxilScalarizeVectorIntrinsics::ID = 0;

ModulePass *llvm::createDxilScalarizeVectorIntrinsicsPass() {
  return new DxilScalarizeVectorIntrinsics();
}

INITIALIZE_PASS(
    DxilScalarizeVectorIntrinsics, "hlsl-dxil-scalarize-vector-intrinsics",
    "Scalarize native vector DXIL loads, stores, and other intrinsics", false,
    false)
