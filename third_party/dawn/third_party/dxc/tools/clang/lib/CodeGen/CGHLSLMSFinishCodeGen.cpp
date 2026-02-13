///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// CGHLSLMSFinishCodeGen.cpp                                                 //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
//  Impliment FinishCodeGen.                                                 //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/DxilValueCache.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GetElementPtrTypeIterator.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

#include "CodeGenFunction.h"
#include "CodeGenModule.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "clang/Parse/ParseHLSL.h" // root sig would be in Parser if part of lang
#include "clang/Sema/SemaDiagnostic.h"

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilResourceProperties.h"
#include "dxc/DXIL/DxilTypeSystem.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/DxilRootSignature/DxilRootSignature.h"
#include "dxc/HLSL/DxilExportMap.h"
#include "dxc/HLSL/DxilGenerationPass.h"
#include "dxc/HLSL/HLMatrixType.h"
#include "dxc/HLSL/HLModule.h"
#include "dxc/HLSL/HLSLExtensionsCodegenHelper.h"
#include "dxc/HlslIntrinsicOp.h"

#include <fenv.h>
#include <memory>
#include <vector>

#include "CGHLSLMSHelper.h"

using namespace llvm;
using namespace hlsl;
using namespace CGHLSLMSHelper;

namespace {

template <typename BuilderTy>
Value *EmitHLOperationCall(HLModule &HLM, BuilderTy &Builder,
                           HLOpcodeGroup group, unsigned opcode, Type *RetType,
                           ArrayRef<Value *> paramList, llvm::Module &M) {
  // Add the opcode param
  llvm::Type *opcodeTy = llvm::Type::getInt32Ty(M.getContext());

  Function *opFunc =
      HLM.GetHLOperationFunction(group, opcode, RetType, paramList, M);

  SmallVector<Value *, 4> opcodeParamList;
  Value *opcodeConst = Constant::getIntegerValue(opcodeTy, APInt(32, opcode));
  opcodeParamList.emplace_back(opcodeConst);
  opcodeParamList.append(paramList.begin(), paramList.end());

  return Builder.CreateCall(opFunc, opcodeParamList);
}

template <typename BuilderTy>
Value *CreateHandleFromResPtr(Value *ResPtr, HLModule &HLM,
                              llvm::Type *HandleTy, BuilderTy &Builder) {
  Module &M = *HLM.GetModule();
  // Load to make sure resource only have Ld/St use so mem2reg could remove
  // temp resource.
  Value *ldObj = Builder.CreateLoad(ResPtr);
  Value *args[] = {ldObj};
  Value *Handle = EmitHLOperationCall<BuilderTy>(
      HLM, Builder, HLOpcodeGroup::HLCreateHandle, 0, HandleTy, args, M);
  return Handle;
}

Value *CreateNodeOutputHandle(HLModule &HLM, llvm::Type *HandleTy,
                              IRBuilder<> &Builder, unsigned index) {
  Module &M = *HLM.GetModule();
  HLOpcodeGroup opCodeGroup = HLOpcodeGroup::HLCreateNodeOutputHandle;
  unsigned opCode =
      static_cast<unsigned>(HLOpcodeGroup::HLCreateNodeOutputHandle);
  Value *mdIndex = Builder.getInt32(index);
  Value *args[] = {mdIndex};
  CallInst *Handle =
      HLM.EmitHLOperationCall(Builder, opCodeGroup, opCode, HandleTy, args, M);
  return Handle;
}

Value *CreateAnnotateNodeHandle(HLModule &HLM, Value *NodeHandle,
                                IRBuilder<> &Builder, NodeInfo Info) {
  llvm::Type *NodeHandleTy = HLM.GetOP()->GetNodeHandleType();
  llvm::Type *NodeInfoTy = HLM.GetOP()->GetNodePropertiesType();
  Module &M = *HLM.GetModule();
  StructType *ST = cast<StructType>(NodeInfoTy);
  Constant *NodeProperties[] = {
      ConstantInt::get(ST->getElementType(0), Info.IOFlags),
      ConstantInt::get(ST->getElementType(1), Info.RecordSize)};
  Constant *NodeInfo = ConstantStruct::get(ST, NodeProperties);
  Value *args[] = {NodeHandle, NodeInfo};
  Value *Handle = HLM.EmitHLOperationCall(
      Builder, HLOpcodeGroup::HLAnnotateNodeHandle,
      static_cast<unsigned>(HLOpcodeGroup::HLAnnotateNodeHandle), NodeHandleTy,
      args, M);
  return Handle;
}

Value *CreateNodeInputRecordHandle(Value *NodeArg, HLModule &HLM,
                                   llvm::Type *HandleTy, IRBuilder<> &Builder,
                                   unsigned index) {
  Module &M = *HLM.GetModule();
  HLOpcodeGroup opCodeGroup = HLOpcodeGroup::HLCreateNodeInputRecordHandle;
  auto opCode =
      static_cast<unsigned>(HLOpcodeGroup::HLCreateNodeInputRecordHandle);
  DXASSERT_NOMSG(index == 0);
  Value *mdIndex = Builder.getInt32(index);
  Value *args[] = {mdIndex};
  CallInst *Handle =
      HLM.EmitHLOperationCall(Builder, opCodeGroup, opCode, HandleTy, args, M);
  return Handle;
}

Value *CreateAnnotateNodeRecordHandle(HLModule &HLM, Value *NodeRecordHandle,
                                      IRBuilder<> &Builder,
                                      NodeRecordInfo Info) {
  llvm::Type *NodeRecordHandleTy = HLM.GetOP()->GetNodeRecordHandleType();
  llvm::Type *NodeRecordInfoTy = HLM.GetOP()->GetNodeRecordPropertiesType();
  Module &M = *HLM.GetModule();
  StructType *ST = cast<StructType>(NodeRecordInfoTy);
  Constant *NodeRecordProperties[] = {
      ConstantInt::get(ST->getElementType(0), Info.IOFlags),
      ConstantInt::get(ST->getElementType(1), Info.RecordSize)};
  Constant *NodeRecordInfo = ConstantStruct::get(ST, NodeRecordProperties);
  Value *args[] = {NodeRecordHandle, NodeRecordInfo};
  Value *Handle = HLM.EmitHLOperationCall(
      Builder, HLOpcodeGroup::HLAnnotateNodeRecordHandle,
      static_cast<unsigned>(HLOpcodeGroup::HLAnnotateNodeRecordHandle),
      NodeRecordHandleTy, args, M);
  return Handle;
}

template <typename BuilderTy>
Value *CreateAnnotateHandle(HLModule &HLM, Value *Handle,
                            DxilResourceProperties &RP, llvm::Type *ResTy,
                            BuilderTy &Builder) {
  Constant *RPConstant = resource_helper::getAsConstant(
      RP, HLM.GetOP()->GetResourcePropertiesType(), *HLM.GetShaderModel());
  return EmitHLOperationCall<BuilderTy>(
      HLM, Builder, HLOpcodeGroup::HLAnnotateHandle,
      (unsigned)HLOpcodeGroup::HLAnnotateHandle, Handle->getType(),
      {Handle, RPConstant, UndefValue::get(ResTy)}, *HLM.GetModule());
}

template <typename BuilderTy>
Value *CastHandleToRes(HLModule &HLM, Value *Handle, llvm::Type *ResTy,
                       BuilderTy &Builder) {
  Value *Res =
      EmitHLOperationCall<BuilderTy>(HLM, Builder, HLOpcodeGroup::HLCast,
                                     (unsigned)HLCastOpcode::HandleToResCast,
                                     ResTy, {Handle}, *HLM.GetModule());
  return Res;
}

// Lower CBV bitcast use to handle use.
// Leave the load/store.
void LowerDynamicCBVUseToHandle(HLModule &HLM,
                                DxilObjectProperties &objectProperties) {
  Type *HandleTy = HLM.GetOP()->GetHandleType();
  Module &M = *HLM.GetModule();
  // Collect BitCast use of CBV.
  SmallVector<std::pair<BitCastInst *, DxilResourceProperties>, 4> BitCasts;
  for (auto it : objectProperties.resMap) {
    DxilResourceProperties RP = it.second;
    if (RP.getResourceKind() != DXIL::ResourceKind::CBuffer &&
        RP.getResourceKind() != DXIL::ResourceKind::TBuffer)
      continue;
    Value *V = it.first;
    // Skip external globals.
    if (GlobalVariable *GV = dyn_cast<GlobalVariable>(V)) {
      if (GV->getLinkage() != GlobalValue::LinkageTypes::InternalLinkage)
        continue;
    }
    for (auto UserIt = V->user_begin(); UserIt != V->user_end();) {
      User *U = *(UserIt++);
      if (U->user_empty())
        continue;
      if (BitCastInst *BCI = dyn_cast<BitCastInst>(U)) {
        BitCasts.emplace_back(std::make_pair(BCI, RP));
        continue;
      }
      DXASSERT((!isa<BitCastOperator>(U) || U->user_empty()),
               "all BitCast should be BitCastInst");
    }
  }

  for (auto it : BitCasts) {
    BitCastInst *BCI = it.first;
    DxilResourceProperties RP = it.second;
    IRBuilder<> B(BCI);
    B.AllowFolding = false;
    Value *ObjV = BCI->getOperand(0);
    Value *Handle = CreateHandleFromResPtr(ObjV, HLM, HandleTy, B);
    Type *ResTy = ObjV->getType()->getPointerElementType();
    Handle = CreateAnnotateHandle(HLM, Handle, RP, ResTy, B);
    // Create cb subscript.
    llvm::Type *opcodeTy = B.getInt32Ty();
    llvm::Type *idxTy = opcodeTy;
    Constant *zeroIdx = ConstantInt::get(opcodeTy, 0);

    Type *cbTy = BCI->getType();
    llvm::FunctionType *SubscriptFuncTy =
        llvm::FunctionType::get(cbTy, {opcodeTy, HandleTy, idxTy}, false);

    Function *subscriptFunc =
        GetOrCreateHLFunction(M, SubscriptFuncTy, HLOpcodeGroup::HLSubscript,
                              (unsigned)HLSubscriptOpcode::CBufferSubscript);
    Constant *opArg = ConstantInt::get(
        opcodeTy, (unsigned)HLSubscriptOpcode::CBufferSubscript);
    Value *args[] = {opArg, Handle, zeroIdx};

    Instruction *cbSubscript =
        cast<Instruction>(B.CreateCall(subscriptFunc, {args}));
    BCI->replaceAllUsesWith(cbSubscript);
    BCI->eraseFromParent();
  }
}

bool IsHLSLSamplerDescType(llvm::Type *Ty) {
  if (llvm::StructType *ST = dyn_cast<llvm::StructType>(Ty)) {
    if (!ST->hasName())
      return false;
    StringRef name = ST->getName();

    if (name == "struct..Sampler")
      return true;
  }
  return false;
}

#ifndef NDEBUG
static bool ConsumePrefix(StringRef &Str, StringRef Prefix) {
  if (!Str.startswith(Prefix))
    return false;
  Str = Str.substr(Prefix.size());
  return true;
}

bool IsHLSLBufferViewType(llvm::Type *Ty) {
  if (llvm::StructType *ST = dyn_cast<llvm::StructType>(Ty)) {
    if (!ST->hasName())
      return false;

    StringRef name = ST->getName();
    if (!(ConsumePrefix(name, "class.") || ConsumePrefix(name, "struct.")))
      return false;

    if (name.startswith("ConstantBuffer<") || name.startswith("TextureBuffer<"))
      return true;
  }
  return false;
}
#endif

void LowerGetResourceFromHeap(
    HLModule &HLM, std::vector<std::pair<Function *, unsigned>> &intrinsicMap) {
  llvm::Module &M = *HLM.GetModule();
  llvm::Type *HandleTy = HLM.GetOP()->GetHandleType();
  unsigned GetResFromHeapOp =
      static_cast<unsigned>(IntrinsicOp::IOP_CreateResourceFromHeap);
  DenseMap<Instruction *, Instruction *> ResourcePtrToHandlePtrMap;

  for (auto it : intrinsicMap) {
    unsigned opcode = it.second;
    if (opcode != GetResFromHeapOp)
      continue;
    Function *F = it.first;
    HLOpcodeGroup group = hlsl::GetHLOpcodeGroup(F);
    if (group != HLOpcodeGroup::HLIntrinsic)
      continue;
    for (auto uit = F->user_begin(); uit != F->user_end();) {
      CallInst *CI = cast<CallInst>(*(uit++));
      // Arg 0 is this pointer.
      unsigned ArgIdx = 1;
      Instruction *ResPtr = cast<Instruction>(CI->getArgOperand(ArgIdx));
      Value *Index = CI->getArgOperand(ArgIdx + 1);
      IRBuilder<> Builder(CI);
      // Make a handle from GetResFromHeap.
      Value *IsSampler = Builder.getInt1(
          IsHLSLSamplerDescType(ResPtr->getType()->getPointerElementType()));
      Value *Handle = HLM.EmitHLOperationCall(
          Builder, HLOpcodeGroup::HLIntrinsic, GetResFromHeapOp, HandleTy,
          {Index, IsSampler}, M);

      // Find the handle ptr for res ptr.
      auto it = ResourcePtrToHandlePtrMap.find(ResPtr);
      Instruction *HandlePtr = nullptr;
      if (it != ResourcePtrToHandlePtrMap.end()) {
        HandlePtr = it->second;
      } else {
        IRBuilder<> AllocaBuilder(
            ResPtr->getParent()->getParent()->getEntryBlock().begin());
        HandlePtr = AllocaBuilder.CreateAlloca(HandleTy);
        ResourcePtrToHandlePtrMap[ResPtr] = HandlePtr;
      }
      // Store handle to handle ptr.
      Builder.CreateStore(Handle, HandlePtr);
      CI->eraseFromParent();
    }
  }

  // Replace load of Resource ptr into load of handel ptr.
  for (auto it : ResourcePtrToHandlePtrMap) {
    Instruction *resPtr = it.first;
    Instruction *handlePtr = it.second;

    for (auto uit = resPtr->user_begin(); uit != resPtr->user_end();) {
      User *U = *(uit++);
      BitCastInst *BCI = cast<BitCastInst>(U);
      DXASSERT(
          dxilutil::IsHLSLResourceType(
              BCI->getType()->getPointerElementType()) ||
              IsHLSLBufferViewType(BCI->getType()->getPointerElementType()),
          "illegal cast of resource ptr");
      for (auto cuit = BCI->user_begin(); cuit != BCI->user_end();) {
        LoadInst *LI = cast<LoadInst>(*(cuit++));
        IRBuilder<> Builder(LI);
        Value *Handle = Builder.CreateLoad(handlePtr);
        Value *Res =
            HLM.EmitHLOperationCall(Builder, HLOpcodeGroup::HLCast,
                                    (unsigned)HLCastOpcode::HandleToResCast,
                                    LI->getType(), {Handle}, M);
        LI->replaceAllUsesWith(Res);
        LI->eraseFromParent();
      }
      BCI->eraseFromParent();
    }
    resPtr->eraseFromParent();
  }
}

void ReplaceBoolVectorSubscript(CallInst *CI) {
  Value *Ptr = CI->getArgOperand(0);
  Value *Idx = CI->getArgOperand(1);
  Value *IdxList[] = {ConstantInt::get(Idx->getType(), 0), Idx};

  for (auto It = CI->user_begin(), E = CI->user_end(); It != E;) {
    Instruction *user = cast<Instruction>(*(It++));

    IRBuilder<> Builder(user);
    Value *GEP = Builder.CreateInBoundsGEP(Ptr, IdxList);

    if (LoadInst *LI = dyn_cast<LoadInst>(user)) {
      Value *NewLd = Builder.CreateLoad(GEP);
      Value *cast = Builder.CreateZExt(NewLd, LI->getType());
      LI->replaceAllUsesWith(cast);
      LI->eraseFromParent();
    } else {
      // Must be a store inst here.
      StoreInst *SI = cast<StoreInst>(user);
      Value *V = SI->getValueOperand();
      Value *cast =
          Builder.CreateICmpNE(V, llvm::ConstantInt::get(V->getType(), 0));
      Builder.CreateStore(cast, GEP);
      SI->eraseFromParent();
    }
  }
  CI->eraseFromParent();
}

void ReplaceBoolVectorSubscript(Function *F) {
  for (auto It = F->user_begin(), E = F->user_end(); It != E;) {
    User *user = *(It++);
    CallInst *CI = cast<CallInst>(user);
    ReplaceBoolVectorSubscript(CI);
  }
}

// Returns a valid field annotation (if present) for the matrix type of
// templated resource on matrix type. Example:- AppendStructuredBuffer<float4x4>
// abuf; Return the field annotation of the matrix type in the above decl.
static DxilFieldAnnotation *
GetTemplatedResMatAnnotation(Function *F, unsigned argOpIdx,
                             unsigned matAnnotationIdx) {
  for (User *U : F->users()) {
    if (CallInst *CI = dyn_cast<CallInst>(U)) {
      if (argOpIdx >= CI->getNumArgOperands())
        continue;
      Value *resArg = CI->getArgOperand(argOpIdx);
      Type *resArgTy = resArg->getType();
      if (resArgTy->isPointerTy())
        resArgTy = cast<PointerType>(resArgTy)->getPointerElementType();
      if (isa<StructType>(resArgTy)) {
        DxilTypeSystem &TS = F->getParent()->GetHLModule().GetTypeSystem();
        auto *SA = TS.GetStructAnnotation(cast<StructType>(resArgTy));
        auto *FA = &(SA->GetFieldAnnotation(matAnnotationIdx));
        if (FA && FA->HasMatrixAnnotation()) {
          return FA;
        }
      }
    }
  }

  return nullptr;
}

// Add function body for intrinsic if possible.
Function *CreateOpFunction(llvm::Module &M, Function *F,
                           llvm::FunctionType *funcTy, HLOpcodeGroup group,
                           unsigned opcode) {
  Function *opFunc = nullptr;
  AttributeSet attribs = F->getAttributes().getFnAttributes();
  llvm::Type *opcodeTy = llvm::Type::getInt32Ty(M.getContext());
  if (group == HLOpcodeGroup::HLIntrinsic) {
    IntrinsicOp intriOp = static_cast<IntrinsicOp>(opcode);
    switch (intriOp) {
    case IntrinsicOp::MOP_Append:
    case IntrinsicOp::MOP_Consume: {
      bool bAppend = intriOp == IntrinsicOp::MOP_Append;
      llvm::Type *handleTy = funcTy->getParamType(HLOperandIndex::kHandleOpIdx);
      // Don't generate body for OutputStream::Append.
      if (bAppend && HLModule::IsStreamOutputPtrType(handleTy)) {
        opFunc = GetOrCreateHLFunction(M, funcTy, group, opcode, attribs);
        break;
      }

      opFunc = GetOrCreateHLFunctionWithBody(M, funcTy, group, opcode,
                                             bAppend ? "append" : "consume");
      llvm::Type *counterTy = llvm::Type::getInt32Ty(M.getContext());
      llvm::FunctionType *IncCounterFuncTy =
          llvm::FunctionType::get(counterTy, {opcodeTy, handleTy}, false);
      unsigned counterOpcode =
          bAppend ? (unsigned)IntrinsicOp::MOP_IncrementCounter
                  : (unsigned)IntrinsicOp::MOP_DecrementCounter;
      Function *incCounterFunc = GetOrCreateHLFunction(
          M, IncCounterFuncTy, group, counterOpcode, attribs);

      llvm::Type *idxTy = counterTy;
      llvm::Type *valTy =
          bAppend ? funcTy->getParamType(HLOperandIndex::kAppendValOpIndex)
                  : funcTy->getReturnType();

      // Return type for subscript should be pointer type, hence in memory
      // representation
      llvm::Type *subscriptTy = valTy;
      bool isBoolScalarOrVector = false;
      if (!subscriptTy->isPointerTy()) {
        if (subscriptTy->getScalarType()->isIntegerTy(1)) {
          isBoolScalarOrVector = true;
          llvm::Type *memReprType =
              llvm::IntegerType::get(subscriptTy->getContext(), 32);
          subscriptTy =
              subscriptTy->isVectorTy()
                  ? llvm::VectorType::get(memReprType,
                                          subscriptTy->getVectorNumElements())
                  : memReprType;
        }
        subscriptTy = llvm::PointerType::get(subscriptTy, 0);
      }

      llvm::FunctionType *SubscriptFuncTy = llvm::FunctionType::get(
          subscriptTy, {opcodeTy, handleTy, idxTy}, false);

      Function *subscriptFunc = GetOrCreateHLFunction(
          M, SubscriptFuncTy, HLOpcodeGroup::HLSubscript,
          (unsigned)HLSubscriptOpcode::DefaultSubscript, attribs);

      BasicBlock *BB =
          BasicBlock::Create(opFunc->getContext(), "Entry", opFunc);
      IRBuilder<> Builder(BB);
      auto argIter = opFunc->args().begin();
      // Skip the opcode arg.
      argIter++;
      Argument *thisArg = argIter++;
      // int counter = IncrementCounter/DecrementCounter(Buf);
      Value *incCounterOpArg = ConstantInt::get(idxTy, counterOpcode);
      Value *counter =
          Builder.CreateCall(incCounterFunc, {incCounterOpArg, thisArg});
      // Buf[counter];
      Value *subscriptOpArg = ConstantInt::get(
          idxTy, (unsigned)HLSubscriptOpcode::DefaultSubscript);
      Value *subscript =
          Builder.CreateCall(subscriptFunc, {subscriptOpArg, thisArg, counter});

      constexpr unsigned kArgIdx = 0;
      constexpr unsigned kMatAnnotationIdx = 0;
      DxilFieldAnnotation *MatAnnotation =
          HLMatrixType::isa(valTy)
              ? GetTemplatedResMatAnnotation(F, kArgIdx, kMatAnnotationIdx)
              : nullptr;
      if (bAppend) {
        Argument *valArg = argIter;
        // Buf[counter] = val;
        if (valTy->isPointerTy()) {
          unsigned size = M.getDataLayout().getTypeAllocSize(
              subscript->getType()->getPointerElementType());
          Builder.CreateMemCpy(subscript, valArg, size, 1);
        } else if (MatAnnotation) {
          // If the to-be-stored value is a matrix then we need to generate
          // an HL matrix store which is then handled appropriately in
          // HLMatrixLowerPass.
          bool isRowMajor = MatAnnotation->GetMatrixAnnotation().Orientation ==
                            MatrixOrientation::RowMajor;
          Value *matStoreVal = valArg;

          // The in-reg matrix orientation is always row-major.
          // If the in-memory matrix orientation is col-major, then we
          // need to change the orientation to col-major before storing
          // to memory
          if (!isRowMajor) {
            unsigned castOpCode = (unsigned)HLCastOpcode::RowMatrixToColMatrix;

            // Construct signature of the function that is used for converting
            // orientation of a matrix from row-major to col-major.
            FunctionType *MatCastFnType = FunctionType::get(
                matStoreVal->getType(),
                {Builder.getInt32Ty(), matStoreVal->getType()},
                /* isVarArg */ false);

            // Create the conversion function.
            Function *MatCastFn = GetOrCreateHLFunction(
                M, MatCastFnType, HLOpcodeGroup::HLCast, castOpCode);
            Value *MatCastOpCode =
                ConstantInt::get(Builder.getInt32Ty(), castOpCode);

            // Insert call to the conversion function.
            matStoreVal =
                Builder.CreateCall(MatCastFn, {MatCastOpCode, matStoreVal});
          }

          unsigned storeOpCode =
              isRowMajor ? (unsigned)HLMatLoadStoreOpcode::RowMatStore
                         : (unsigned)HLMatLoadStoreOpcode::ColMatStore;

          // Construct signature of the function that is used for storing
          // the matrix value to the memory.
          FunctionType *MatStFnType = FunctionType::get(
              Builder.getVoidTy(),
              {Builder.getInt32Ty(), subscriptTy, matStoreVal->getType()},
              /* isVarArg */ false);

          // Create the matrix store function.
          Function *MatStFn = GetOrCreateHLFunction(
              M, MatStFnType, HLOpcodeGroup::HLMatLoadStore, storeOpCode);
          Value *MatStOpCode =
              ConstantInt::get(Builder.getInt32Ty(), storeOpCode);

          // Insert call to the matrix store function.
          Builder.CreateCall(MatStFn, {MatStOpCode, subscript, matStoreVal});
        } else {
          Value *storedVal = valArg;
          // Convert to memory representation
          if (isBoolScalarOrVector)
            storedVal = Builder.CreateZExt(
                storedVal, subscriptTy->getPointerElementType(), "frombool");
          Builder.CreateStore(storedVal, subscript);
        }
        Builder.CreateRetVoid();
      } else {
        // return Buf[counter];
        if (valTy->isPointerTy())
          Builder.CreateRet(subscript);
        else if (MatAnnotation) {
          // If the to-be-loaded value is a matrix then we need to generate
          // an HL matrix load which is then handled appropriately in
          // HLMatrixLowerPass.
          bool isRowMajor = MatAnnotation->GetMatrixAnnotation().Orientation ==
                            MatrixOrientation::RowMajor;

          unsigned loadOpCode =
              isRowMajor ? (unsigned)HLMatLoadStoreOpcode::RowMatLoad
                         : (unsigned)HLMatLoadStoreOpcode::ColMatLoad;

          // Construct signature of the function that is used for loading
          // the matrix value from the memory.
          FunctionType *MatLdFnType =
              FunctionType::get(valTy, {Builder.getInt32Ty(), subscriptTy},
                                /* isVarArg */ false);

          // Create the matrix load function.
          Function *MatLdFn = GetOrCreateHLFunction(
              M, MatLdFnType, HLOpcodeGroup::HLMatLoadStore, loadOpCode);
          Value *MatStOpCode =
              ConstantInt::get(Builder.getInt32Ty(), loadOpCode);

          // Insert call to the matrix load function.
          Value *matLdVal =
              Builder.CreateCall(MatLdFn, {MatStOpCode, subscript});

          // The in-reg matrix orientation is always row-major.
          // If the in-memory matrix orientation is col-major, then we
          // need to change the orientation to row-major after loading
          // from memory.
          if (!isRowMajor) {
            unsigned castOpCode = (unsigned)HLCastOpcode::ColMatrixToRowMatrix;

            // Construct signature of the function that is used for converting
            // orientation of a matrix from col-major to row-major.
            FunctionType *MatCastFnType =
                FunctionType::get(matLdVal->getType(),
                                  {Builder.getInt32Ty(), matLdVal->getType()},
                                  /* isVarArg */ false);

            // Create the conversion function.
            Function *MatCastFn = GetOrCreateHLFunction(
                M, MatCastFnType, HLOpcodeGroup::HLCast, castOpCode);
            Value *MatCastOpCode =
                ConstantInt::get(Builder.getInt32Ty(), castOpCode);

            // Insert call to the conversion function.
            matLdVal = Builder.CreateCall(MatCastFn, {MatCastOpCode, matLdVal});
          }
          Builder.CreateRet(matLdVal);
        } else {
          Value *retVal = Builder.CreateLoad(subscript);
          // Convert to register representation
          if (isBoolScalarOrVector)
            retVal = Builder.CreateICmpNE(
                retVal, Constant::getNullValue(retVal->getType()), "tobool");
          Builder.CreateRet(retVal);
        }
      }
    } break;
    case IntrinsicOp::IOP_sincos: {
      opFunc =
          GetOrCreateHLFunctionWithBody(M, funcTy, group, opcode, "sincos");
      llvm::Type *valTy =
          funcTy->getParamType(HLOperandIndex::kTrinaryOpSrc0Idx);

      llvm::FunctionType *sinFuncTy =
          llvm::FunctionType::get(valTy, {opcodeTy, valTy}, false);
      unsigned sinOp = static_cast<unsigned>(IntrinsicOp::IOP_sin);
      unsigned cosOp = static_cast<unsigned>(IntrinsicOp::IOP_cos);
      Function *sinFunc =
          GetOrCreateHLFunction(M, sinFuncTy, group, sinOp, attribs);
      Function *cosFunc =
          GetOrCreateHLFunction(M, sinFuncTy, group, cosOp, attribs);

      BasicBlock *BB =
          BasicBlock::Create(opFunc->getContext(), "Entry", opFunc);
      IRBuilder<> Builder(BB);
      auto argIter = opFunc->args().begin();
      // Skip the opcode arg.
      argIter++;
      Argument *valArg = argIter++;
      Argument *sinPtrArg = argIter++;
      Argument *cosPtrArg = argIter++;

      Value *sinOpArg = ConstantInt::get(opcodeTy, sinOp);
      Value *sinVal = Builder.CreateCall(sinFunc, {sinOpArg, valArg});
      Builder.CreateStore(sinVal, sinPtrArg);

      Value *cosOpArg = ConstantInt::get(opcodeTy, cosOp);
      Value *cosVal = Builder.CreateCall(cosFunc, {cosOpArg, valArg});
      Builder.CreateStore(cosVal, cosPtrArg);
      // Ret.
      Builder.CreateRetVoid();
    } break;
    default:
      opFunc = GetOrCreateHLFunction(M, funcTy, group, opcode, attribs);
      break;
    }
  } else if (group == HLOpcodeGroup::HLExtIntrinsic) {
    llvm::StringRef fnName = F->getName();
    llvm::StringRef groupName = GetHLOpcodeGroupNameByAttr(F);
    opFunc = GetOrCreateHLFunction(M, funcTy, group, &groupName, &fnName,
                                   opcode, attribs);
  } else {
    opFunc = GetOrCreateHLFunction(M, funcTy, group, opcode, attribs);
  }

  return opFunc;
}

DxilResourceProperties
GetResourcePropsFromIntrinsicObjectArg(Value *arg, HLModule &HLM,
                                       DxilTypeSystem &typeSys,
                                       DxilObjectProperties &objectProperties) {
  DxilResourceProperties RP = objectProperties.GetResource(arg);
  if (RP.isValid())
    return RP;

  // Must be GEP.
  GEPOperator *GEP = cast<GEPOperator>(arg);
  // Find RP from GEP.
  Value *Ptr = GEP->getPointerOperand();
  // When Ptr is array of resource, check if it is another GEP.
  while (
      dxilutil::IsHLSLResourceType(dxilutil::GetArrayEltTy(Ptr->getType()))) {
    if (GEPOperator *ParentGEP = dyn_cast<GEPOperator>(Ptr)) {
      GEP = ParentGEP;
      Ptr = GEP->getPointerOperand();
    } else {
      break;
    }
  }

  // When ptr is array of resource, ptr could be in
  // objectProperties.
  RP = objectProperties.GetResource(Ptr);
  if (RP.isValid())
    return RP;

  DxilStructAnnotation *Anno = nullptr;

  for (auto gepIt = gep_type_begin(GEP), E = gep_type_end(GEP); gepIt != E;
       ++gepIt) {

    if (StructType *ST = dyn_cast<StructType>(*gepIt)) {
      Anno = typeSys.GetStructAnnotation(ST);
      DXASSERT(Anno, "missing type annotation");

      unsigned Index = cast<ConstantInt>(gepIt.getOperand())->getLimitedValue();

      DxilFieldAnnotation &fieldAnno = Anno->GetFieldAnnotation(Index);
      if (fieldAnno.HasResourceProperties()) {
        RP = fieldAnno.GetResourceProperties();
        break;
      }
    }
  }
  DXASSERT(RP.isValid(), "invalid resource properties");
  return RP;
}

void AddOpcodeParamForIntrinsic(HLModule &HLM, Function *F, unsigned opcode,
                                DxilObjectProperties &objectProperties) {
  llvm::Module &M = *HLM.GetModule();
  llvm::FunctionType *oldFuncTy = F->getFunctionType();
  llvm::Type *HandleTy = HLM.GetOP()->GetHandleType();
  llvm::Type *NodeRecordHandleTy = HLM.GetOP()->GetNodeRecordHandleType();
  llvm::Type *NodeOutputHandleTy = HLM.GetOP()->GetNodeHandleType();

  HLOpcodeGroup group = hlsl::GetHLOpcodeGroup(F);

  SmallVector<llvm::Type *, 4> paramTyList;
  // Add the opcode param
  llvm::Type *opcodeTy = llvm::Type::getInt32Ty(M.getContext());
  paramTyList.emplace_back(opcodeTy);

  // Create a vector of the types of the original Intrinsic's
  // parameters. In the case of an intrinsic returning a struct
  bool bSRetHandle = false;
  bool bNodeRecordRet = false;
  bool bNodeOutputRet = false;
  bool bNodeOutputMethod = false;
  bool bNodeOutputArrayMethod = false;
  unsigned bRetArgIndex = 0;
  for (unsigned i = 0; i < oldFuncTy->getNumParams(); i++) {
    llvm::Type *Ty = oldFuncTy->getParamType(i);

    if (i == 0 && Ty->isPointerTy() &&
        dxilutil::IsHLSLNodeOutputType(Ty->getPointerElementType()))
      bNodeOutputMethod = true;

    if (i == 0 && Ty->isPointerTy() &&
        dxilutil::IsHLSLNodeOutputArrayType(Ty->getPointerElementType()))
      bNodeOutputArrayMethod = true;

    if (i == 1 && bNodeOutputMethod && Ty->isPointerTy() &&
        dxilutil::IsHLSLNodeOutputRecordType(Ty->getPointerElementType())) {
      // Skip for return type from a method.
      // NodeOutput<recType>::GetGroupNodeOutputRecords
      // NodeOutput<recType>::GetThreadNodeOutputRecords
      bSRetHandle = true;
      bNodeRecordRet = true;
      bRetArgIndex = 1;
      continue;
    }

    if (i == 1 && bNodeOutputArrayMethod && Ty->isPointerTy() &&
        dxilutil::IsHLSLNodeOutputType(Ty->getPointerElementType())) {
      // Skip for return type from a method
      bSRetHandle = true;
      bNodeOutputRet = true;
      bRetArgIndex = 1;
      continue;
    }

    if (Ty->isPointerTy()) {
      llvm::Type *PtrEltTy = Ty->getPointerElementType();

      if (dxilutil::IsHLSLResourceType(PtrEltTy) ||
          dxilutil::IsHLSLNodeRecordType(PtrEltTy) ||
          dxilutil::IsHLSLNodeOutputType(PtrEltTy) ||
          dxilutil::IsHLSLNodeOutputArrayType(PtrEltTy)) {
        // Skip for return type.
        if (i == 0 && F->arg_begin()->hasStructRetAttr()) {
          bSRetHandle = true;
          if (dxilutil::IsHLSLNodeRecordType(PtrEltTy)) {
            bNodeRecordRet = true;
          }
          continue;
        }
        // Use handle type for resource, Node(Input/Output) Record type.
        // This will make sure temp object variable only used by createHandle.
        if (dxilutil::IsHLSLResourceType(PtrEltTy))
          Ty = HandleTy;
        else if (dxilutil::IsHLSLNodeRecordType(PtrEltTy))
          Ty = NodeRecordHandleTy;
        else if (dxilutil::IsHLSLNodeOutputType(PtrEltTy))
          Ty = NodeOutputHandleTy;
        else if (dxilutil::IsHLSLNodeOutputArrayType(PtrEltTy))
          Ty = NodeOutputHandleTy;
      }
    }
    paramTyList.emplace_back(Ty);
  }

  if (group == HLOpcodeGroup::HLSubscript &&
      opcode == static_cast<unsigned>(HLSubscriptOpcode::VectorSubscript)) {
    llvm::FunctionType *FT = F->getFunctionType();
    llvm::Type *VecArgTy = FT->getParamType(0);
    llvm::VectorType *VType =
        cast<llvm::VectorType>(VecArgTy->getPointerElementType());
    llvm::Type *Ty = VType->getElementType();
    DXASSERT(Ty->isIntegerTy(), "Only bool could use VectorSubscript");
    llvm::IntegerType *ITy = cast<IntegerType>(Ty);

    DXASSERT_LOCALVAR(ITy, ITy->getBitWidth() == 1,
                      "Only bool could use VectorSubscript");

    // The return type is i8*.
    // Replace all uses with i1*.
    ReplaceBoolVectorSubscript(F);
    return;
  }

  bool isDoubleSubscriptFunc =
      group == HLOpcodeGroup::HLSubscript &&
      opcode == static_cast<unsigned>(HLSubscriptOpcode::DoubleSubscript);

  llvm::Type *RetTy = oldFuncTy->getReturnType();
  if (bSRetHandle) {
    DXASSERT(RetTy->isVoidTy(), "else invalid return type");
    RetTy = HandleTy;
    if (bNodeRecordRet)
      RetTy = NodeRecordHandleTy;
    else if (bNodeOutputRet)
      RetTy = NodeOutputHandleTy;
  }

  if (isDoubleSubscriptFunc) {
    CallInst *doubleSub = cast<CallInst>(*F->user_begin());

    // Change currentIdx type into coord type.
    auto U = doubleSub->user_begin();
    Value *user = *U;
    CallInst *secSub = cast<CallInst>(user);
    unsigned coordIdx = HLOperandIndex::kSubscriptIndexOpIdx;
    // opcode operand not add yet, so the index need -1.
    if (GetHLOpcodeGroupByName(secSub->getCalledFunction()) ==
        HLOpcodeGroup::NotHL)
      coordIdx -= 1;

    Value *coord = secSub->getArgOperand(coordIdx);

    llvm::Type *coordTy = coord->getType();
    paramTyList[HLOperandIndex::kSubscriptIndexOpIdx] = coordTy;
    // Add the sampleIdx or mipLevel parameter to the end.
    paramTyList.emplace_back(opcodeTy);
    // Change return type to be resource ret type.
    // opcode operand not add yet, so the index need -1.
    Value *objPtr =
        doubleSub->getArgOperand(HLOperandIndex::kSubscriptObjectOpIdx - 1);
    // Must be a GEP
    GEPOperator *objGEP = cast<GEPOperator>(objPtr);
    gep_type_iterator GEPIt = gep_type_begin(objGEP), E = gep_type_end(objGEP);
    llvm::Type *resTy = nullptr;
    while (GEPIt != E) {
      if (dxilutil::IsHLSLResourceType(*GEPIt)) {
        resTy = *GEPIt;
        break;
      }
      GEPIt++;
    }

    DXASSERT(resTy, "must find the resource type");
    // Change object type to handle type.
    paramTyList[HLOperandIndex::kSubscriptObjectOpIdx] = HandleTy;
    // Change RetTy into pointer of resource return type.
    RetTy = cast<StructType>(resTy)->getElementType(0)->getPointerTo();
  }

  llvm::FunctionType *funcTy =
      llvm::FunctionType::get(RetTy, paramTyList, oldFuncTy->isVarArg());

  Function *opFunc = CreateOpFunction(M, F, funcTy, group, opcode);
  StringRef lower = hlsl::GetHLLowerStrategy(F);
  if (!lower.empty())
    hlsl::SetHLLowerStrategy(opFunc, lower);

  DxilTypeSystem &typeSys = HLM.GetTypeSystem();

  for (auto user = F->user_begin(); user != F->user_end();) {
    // User must be a call.
    CallInst *oldCI = cast<CallInst>(*(user++));

    SmallVector<Value *, 4> opcodeParamList;
    Value *opcodeConst = Constant::getIntegerValue(opcodeTy, APInt(32, opcode));
    opcodeParamList.emplace_back(opcodeConst);
    Value *retHandleArg = nullptr;
    Value *nodeRecordArg = nullptr;
    if (!bSRetHandle) {
      opcodeParamList.append(oldCI->arg_operands().begin(),
                             oldCI->arg_operands().end());
    } else {
      auto it = oldCI->arg_operands().begin();
      unsigned argIndex = 0;
      auto start = it;
      while (argIndex < bRetArgIndex) {
        it++;
        argIndex++;
      }
      opcodeParamList.append(start, it);
      retHandleArg = *(it++);
      opcodeParamList.append(it, oldCI->arg_operands().end());
    }
    IRBuilder<> Builder(oldCI);

    if (isDoubleSubscriptFunc) {
      // Change obj to the resource pointer.
      Value *objVal = opcodeParamList[HLOperandIndex::kSubscriptObjectOpIdx];
      GEPOperator *objGEP = cast<GEPOperator>(objVal);
      SmallVector<Value *, 8> IndexList;
      IndexList.append(objGEP->idx_begin(), objGEP->idx_end());
      Value *lastIndex = IndexList.back();
      ConstantInt *constIndex = cast<ConstantInt>(lastIndex);
      DXASSERT_LOCALVAR(constIndex, constIndex->getLimitedValue() == 1,
                        "last index must 1");
      // Remove the last index.
      IndexList.pop_back();
      objVal = objGEP->getPointerOperand();

      DxilResourceProperties RP = GetResourcePropsFromIntrinsicObjectArg(
          objVal, HLM, typeSys, objectProperties);

      if (IndexList.size() > 1)
        objVal = Builder.CreateInBoundsGEP(objVal, IndexList);

      Value *Handle = CreateHandleFromResPtr(objVal, HLM, HandleTy, Builder);

      Type *ResTy = objVal->getType()->getPointerElementType();
      Handle = CreateAnnotateHandle(HLM, Handle, RP, ResTy, Builder);
      // Change obj to the resource pointer.
      opcodeParamList[HLOperandIndex::kSubscriptObjectOpIdx] = Handle;

      // Set idx and mipIdx.
      Value *mipIdx = opcodeParamList[HLOperandIndex::kSubscriptIndexOpIdx];
      auto U = oldCI->user_begin();
      Value *user = *U;
      CallInst *secSub = cast<CallInst>(user);
      unsigned idxOpIndex = HLOperandIndex::kSubscriptIndexOpIdx;
      if (GetHLOpcodeGroupByName(secSub->getCalledFunction()) ==
          HLOpcodeGroup::NotHL)
        idxOpIndex--;
      Value *idx = secSub->getArgOperand(idxOpIndex);

      DXASSERT(secSub->hasOneUse(), "subscript should only has one use");

      // Add the sampleIdx or mipLevel parameter to the end.
      opcodeParamList[HLOperandIndex::kSubscriptIndexOpIdx] = idx;
      opcodeParamList.emplace_back(mipIdx);
      // Insert new call before secSub to make sure idx is ready to use.
      Builder.SetInsertPoint(secSub);
    }
    unsigned recordSizeWAR = 0;
    for (unsigned i = 1; i < opcodeParamList.size(); i++) {
      Value *arg = opcodeParamList[i];
      llvm::Type *Ty = arg->getType();
      if (Ty->isPointerTy()) {
        Ty = Ty->getPointerElementType();
        if (dxilutil::IsHLSLResourceType(Ty)) {
          DxilResourceProperties RP = GetResourcePropsFromIntrinsicObjectArg(
              arg, HLM, typeSys, objectProperties);
          // Use object type directly, not by pointer.
          // This will make sure temp object variable only used by ld/st.
          if (GEPOperator *argGEP = dyn_cast<GEPOperator>(arg)) {
            std::vector<Value *> idxList(argGEP->idx_begin(),
                                         argGEP->idx_end());
            // Create instruction to avoid GEPOperator.
            GetElementPtrInst *GEP = GetElementPtrInst::CreateInBounds(
                argGEP->getPointerOperand(), idxList);
            Builder.Insert(GEP);
            arg = GEP;
          }

          llvm::Type *ResTy = arg->getType()->getPointerElementType();

          Value *Handle = CreateHandleFromResPtr(arg, HLM, HandleTy, Builder);
          Handle = CreateAnnotateHandle(HLM, Handle, RP, ResTy, Builder);
          opcodeParamList[i] = Handle;
        }
        if (dxilutil::IsHLSLNodeRecordType(Ty)) {
          nodeRecordArg = arg;
          Value *ldObj = Builder.CreateLoad(arg);
          Value *Handle = EmitHLOperationCall(
              HLM, Builder, HLOpcodeGroup::HLCast,
              (unsigned)HLCastOpcode::NodeRecordToHandleCast,
              NodeRecordHandleTy, {ldObj}, *HLM.GetModule());
          opcodeParamList[i] = Handle;
        }
        if (dxilutil::IsHLSLNodeOutputArrayType(Ty)) {
          Value *ldObj = Builder.CreateLoad(arg);
          Value *Handle = EmitHLOperationCall(
              HLM, Builder, HLOpcodeGroup::HLCast,
              (unsigned)HLCastOpcode::NodeOutputToHandleCast,
              NodeOutputHandleTy, {ldObj}, *HLM.GetModule());
          opcodeParamList[i] = Handle;
          // WAR for record size computation
          if (!dxilutil::IsHLSLEmptyNodeOutputArrayType(Ty)) {
            DxilStructAnnotation *pAnno =
                HLM.GetTypeSystem().GetStructAnnotation(
                    dyn_cast<llvm::StructType>(Ty));
            assert(pAnno != nullptr && pAnno->GetNumTemplateArgs() == 1 &&
                   "otherwise the node template is not declared properly");
            llvm::Type *pRecType = const_cast<llvm::Type *>(
                pAnno->GetTemplateArgAnnotation(0).GetType());
            recordSizeWAR = M.getDataLayout().getTypeAllocSize(pRecType);
          }
        }
        if (dxilutil::IsHLSLNodeOutputType(Ty)) {
          Value *ldObj = Builder.CreateLoad(arg);
          Value *Handle = EmitHLOperationCall(
              HLM, Builder, HLOpcodeGroup::HLCast,
              (unsigned)HLCastOpcode::NodeOutputToHandleCast,
              NodeOutputHandleTy, {ldObj}, *HLM.GetModule());
          opcodeParamList[i] = Handle;
        }
      }
    }

    Value *CI = Builder.CreateCall(opFunc, opcodeParamList);
    if (group == HLOpcodeGroup::HLIntrinsic &&
        opcode == static_cast<unsigned>(IntrinsicOp::MOP_OutputComplete)) {
      DXASSERT_NOMSG(nodeRecordArg->getType()->isPointerTy());
      Type *T = nodeRecordArg->getType()->getPointerElementType();
      Builder.CreateStore(Constant::getNullValue(T), nodeRecordArg);
    }
    if (retHandleArg) {
      Type *ResTy = retHandleArg->getType()->getPointerElementType();
      if (dxilutil::IsHLSLNodeOutputRecordType(ResTy)) {
        CallInst *GetOpRecordCall = cast<CallInst>(CI);
        DXASSERT_NOMSG(group == HLOpcodeGroup::HLIntrinsic);
        IntrinsicOp opcode =
            static_cast<IntrinsicOp>(hlsl::GetHLOpcode(GetOpRecordCall));
        DXIL::NodeIOKind kind;
        if (opcode == IntrinsicOp::MOP_GetGroupNodeOutputRecords)
          kind = DXIL::NodeIOKind::GroupNodeOutputRecords;
        else
          kind = DXIL::NodeIOKind::ThreadNodeOutputRecords;

        // Get record size from the node output record template argument

        DxilStructAnnotation *pAnno = HLM.GetTypeSystem().GetStructAnnotation(
            dyn_cast<llvm::StructType>(ResTy));
        assert(pAnno != nullptr && pAnno->GetNumTemplateArgs() == 1 &&
               "otherwise the node record template is not declared properly");
        llvm::Type *pRecType = const_cast<llvm::Type *>(
            pAnno->GetTemplateArgAnnotation(0).GetType());
        unsigned recordSize = M.getDataLayout().getTypeAllocSize(pRecType);
        NodeRecordInfo Info(kind, recordSize);

        CI = CreateAnnotateNodeRecordHandle(HLM, CI, Builder, Info);
        Value *Res = HLM.EmitHLOperationCall(
            Builder, HLOpcodeGroup::HLCast,
            (unsigned)HLCastOpcode::HandleToNodeRecordCast, ResTy, {CI}, M);
        Builder.CreateStore(Res, retHandleArg);
      } else if (dxilutil::IsHLSLNodeOutputType(ResTy)) {
        DXASSERT_NOMSG(group == HLOpcodeGroup::HLIndexNodeHandle);
        DXIL::NodeIOKind kind = DXIL::NodeIOKind::NodeOutputArray;
        unsigned recordSize;
        if (dxilutil::IsHLSLEmptyNodeOutputType(ResTy)) {
          kind = DXIL::NodeIOKind::EmptyOutputArray;
          recordSize = 0;
        } else {
          // TODO FIX AddStructAnnotation
          /*DxilStructAnnotation* pAnno =
          HLM.GetTypeSystem().GetStructAnnotation(dyn_cast<llvm::StructType>(ResTy));
          assert(pAnno != nullptr && pAnno->GetNumTemplateArgs() == 1 &&
          "otherwise the node template is not declared properly"); llvm::Type*
          pRecType = (llvm::Type*)pAnno->GetTemplateArgAnnotation(0).GetType();
          recordSize = M.getDataLayout().getTypeAllocSize(pRecType);*/
          recordSize = recordSizeWAR;
        }
        NodeInfo Info(kind, recordSize);

        CI = CreateAnnotateNodeHandle(HLM, CI, Builder, Info);
        Value *Res = HLM.EmitHLOperationCall(
            Builder, HLOpcodeGroup::HLCast,
            (unsigned)HLCastOpcode::HandleToNodeOutputCast, ResTy, {CI}, M);
        Builder.CreateStore(Res, retHandleArg);
      } else {
        Value *Res = HLM.EmitHLOperationCall(
            Builder, HLOpcodeGroup::HLCast,
            (unsigned)HLCastOpcode::HandleToResCast, ResTy, {CI}, M);
        Builder.CreateStore(Res, retHandleArg);
      }
      oldCI->eraseFromParent();
      continue;
    }
    if (!isDoubleSubscriptFunc) {
      // replace new call and delete the old call
      oldCI->replaceAllUsesWith(CI);
      oldCI->eraseFromParent();
    } else {
      // For double script.
      // Replace single users use with new CI.
      auto U = oldCI->user_begin();
      Value *user = *U;
      CallInst *secSub = cast<CallInst>(user);
      secSub->replaceAllUsesWith(CI);
      secSub->eraseFromParent();
      oldCI->eraseFromParent();
    }
  }
  // delete the function
  F->eraseFromParent();
}

void AddOpcodeParamForIntrinsics(
    HLModule &HLM, std::vector<std::pair<Function *, unsigned>> &intrinsicMap,
    DxilObjectProperties &objectProperties) {

  for (auto mapIter : intrinsicMap) {
    Function *F = mapIter.first;
    if (F->user_empty()) {
      // delete the function
      F->eraseFromParent();
      continue;
    }

    unsigned opcode = mapIter.second;
    AddOpcodeParamForIntrinsic(HLM, F, opcode, objectProperties);
  }
}

// Returns whether the first argument of CI is NaN or not. If the argument is
// a vector, returns a vector of boolean values.
Constant *IsNaN(CallInst *CI) {
  Value *V = CI->getArgOperand(0);
  llvm::Type *Ty = V->getType();
  if (llvm::VectorType *VT = dyn_cast<llvm::VectorType>(Ty)) {
    Constant *CV = cast<Constant>(V);
    SmallVector<Constant *, 4> ConstVec;
    llvm::Type *CIElemTy =
        cast<llvm::VectorType>(CI->getType())->getElementType();
    for (unsigned i = 0; i < VT->getNumElements(); i++) {
      ConstantFP *fpV = cast<ConstantFP>(CV->getAggregateElement(i));
      bool isNan = fpV->getValueAPF().isNaN();
      ConstVec.push_back(ConstantInt::get(CIElemTy, isNan ? 1 : 0));
    }
    return ConstantVector::get(ConstVec);
  } else {
    ConstantFP *fV = cast<ConstantFP>(V);
    bool isNan = fV->getValueAPF().isNaN();
    return ConstantInt::get(CI->getType(), isNan ? 1 : 0);
  }
}

// Returns a constant for atan2() intrinsic function for scalars.
Constant *Atan2ForScalar(llvm::Type *ResultTy, ConstantFP *fpV0,
                         ConstantFP *fpV1) {
  if (ResultTy->isDoubleTy()) {
    double dV0 = fpV0->getValueAPF().convertToDouble();
    double dV1 = fpV1->getValueAPF().convertToDouble();
    return ConstantFP::get(ResultTy, atan2(dV0, dV1));
  } else {
    DXASSERT_NOMSG(ResultTy->isFloatTy());
    float fV0 = fpV0->getValueAPF().convertToFloat();
    float fV1 = fpV1->getValueAPF().convertToFloat();
    return ConstantFP::get(ResultTy, atan2f(fV0, fV1));
  }
}

// Returns Value for atan2() intrinsic function. If the argument of CI has
// a vector type, it returns the vector value of atan2().
Value *Atan2(CallInst *CI) {
  Value *V0 = CI->getArgOperand(0);
  Value *V1 = CI->getArgOperand(1);
  if (llvm::VectorType *VT = dyn_cast<llvm::VectorType>(V0->getType())) {
    Constant *CV0 = cast<Constant>(V0);
    Constant *CV1 = cast<Constant>(V1);
    SmallVector<Constant *, 4> ConstVec;
    llvm::Type *CIElemTy =
        cast<llvm::VectorType>(CI->getType())->getElementType();
    for (unsigned i = 0; i < VT->getNumElements(); i++) {
      ConstantFP *fpV0 = cast<ConstantFP>(CV0->getAggregateElement(i));
      ConstantFP *fpV1 = cast<ConstantFP>(CV1->getAggregateElement(i));
      ConstVec.push_back(Atan2ForScalar(CIElemTy, fpV0, fpV1));
    }
    return ConstantVector::get(ConstVec);
  } else {
    ConstantFP *fpV0 = cast<ConstantFP>(V0);
    ConstantFP *fpV1 = cast<ConstantFP>(V1);
    return Atan2ForScalar(CI->getType(), fpV0, fpV1);
  }
}
} // namespace

namespace CGHLSLMSHelper {
void CopyAndAnnotateResourceArgument(llvm::Value *Src, llvm::Value *Dest,
                                     DxilResourceProperties &RP,
                                     hlsl::HLModule &HLM,
                                     clang::CodeGen::CodeGenFunction &CGF) {
  Type *ResTy = Src->getType()->getPointerElementType();
  Type *HandleTy = HLM.GetOP()->GetHandleType();
  // Ld resource -> annotateHandle -> turnback to resource.

  Value *Handle = CreateHandleFromResPtr(Src, HLM, HandleTy, CGF.Builder);

  Handle = CreateAnnotateHandle(HLM, Handle, RP, ResTy, CGF.Builder);
  Value *Res = CastHandleToRes(HLM, Handle, ResTy, CGF.Builder);
  CGF.Builder.CreateStore(Res, Dest);
}
} // namespace CGHLSLMSHelper

namespace {

// Returns true a global value is being updated
bool GlobalHasStoreUserRec(Value *V, std::set<Value *> &visited) {
  bool isWriteEnabled = false;
  if (V && visited.find(V) == visited.end()) {
    visited.insert(V);
    for (User *U : V->users()) {
      if (isa<StoreInst>(U)) {
        return true;
      } else if (CallInst *CI = dyn_cast<CallInst>(U)) {
        Function *F = CI->getCalledFunction();
        if (!F->isIntrinsic()) {
          HLOpcodeGroup hlGroup = GetHLOpcodeGroup(F);
          switch (hlGroup) {
          case HLOpcodeGroup::NotHL:
            return true;
          case HLOpcodeGroup::HLMatLoadStore: {
            HLMatLoadStoreOpcode opCode =
                static_cast<HLMatLoadStoreOpcode>(hlsl::GetHLOpcode(CI));
            if (opCode == HLMatLoadStoreOpcode::ColMatStore ||
                opCode == HLMatLoadStoreOpcode::RowMatStore)
              return true;
            break;
          }
          case HLOpcodeGroup::HLCast:
          case HLOpcodeGroup::HLSubscript:
            if (GlobalHasStoreUserRec(U, visited))
              return true;
            break;
          default:
            break;
          }
        }
      } else if (isa<GEPOperator>(U) || isa<PHINode>(U) || isa<SelectInst>(U)) {
        if (GlobalHasStoreUserRec(U, visited))
          return true;
      }
    }
  }
  return isWriteEnabled;
}
// Returns true if any of the direct user of a global is a store inst
// otherwise recurse through the remaining users and check if any GEP
// exists and which in turn has a store inst as user.
bool GlobalHasStoreUser(GlobalVariable *GV) {
  std::set<Value *> visited;
  Value *V = cast<Value>(GV);
  return GlobalHasStoreUserRec(V, visited);
}

GlobalVariable *CreateStaticGlobal(llvm::Module *M, GlobalVariable *GV) {
  Constant *GC = M->getOrInsertGlobal(GV->getName().str() + ".static.copy",
                                      GV->getType()->getPointerElementType());
  GlobalVariable *NGV = cast<GlobalVariable>(GC);
  if (GV->hasInitializer()) {
    NGV->setInitializer(GV->getInitializer());
  } else {
    // The copy being static, it should be initialized per llvm rules
    NGV->setInitializer(
        Constant::getNullValue(GV->getType()->getPointerElementType()));
  }
  // static global should have internal linkage
  NGV->setLinkage(GlobalValue::InternalLinkage);
  return NGV;
}

void CreateWriteEnabledStaticGlobals(llvm::Module *M, llvm::Function *EF) {
  std::vector<GlobalVariable *> worklist;
  for (GlobalVariable &GV : M->globals()) {
    if (!GV.isConstant() && GV.getLinkage() != GlobalValue::InternalLinkage &&
        // skip globals which are HLSL objects or group shared
        !dxilutil::IsHLSLObjectType(GV.getType()->getPointerElementType()) &&
        !dxilutil::IsSharedMemoryGlobal(&GV)) {
      if (GlobalHasStoreUser(&GV))
        worklist.emplace_back(&GV);
      // TODO: Ensure that constant globals aren't using initializer
      GV.setConstant(true);
    }
  }

  IRBuilder<> Builder(
      dxilutil::FirstNonAllocaInsertionPt(&EF->getEntryBlock()));
  for (GlobalVariable *GV : worklist) {
    GlobalVariable *NGV = CreateStaticGlobal(M, GV);
    GV->replaceAllUsesWith(NGV);

    // insert memcpy in all entryblocks
    uint64_t size = M->getDataLayout().getTypeAllocSize(
        GV->getType()->getPointerElementType());
    Builder.CreateMemCpy(NGV, GV, size, 1);
  }
}

} // namespace

namespace {

void SetEntryFunction(HLModule &HLM, Function *Entry,
                      clang::CodeGen::CodeGenModule &CGM) {
  if (Entry == nullptr) {
    clang::DiagnosticsEngine &Diags = CGM.getDiags();
    unsigned DiagID = Diags.getCustomDiagID(clang::DiagnosticsEngine::Error,
                                            "cannot find entry function %0");
    Diags.Report(DiagID) << CGM.getCodeGenOpts().HLSLEntryFunction;
    return;
  }

  HLM.SetEntryFunction(Entry);
}

Function *CloneFunction(Function *Orig, const llvm::Twine &Name,
                        llvm::Module *llvmModule, hlsl::DxilTypeSystem &TypeSys,
                        hlsl::DxilTypeSystem &SrcTypeSys) {

  Function *F = Function::Create(Orig->getFunctionType(),
                                 GlobalValue::LinkageTypes::ExternalLinkage,
                                 Name, llvmModule);

  SmallVector<ReturnInst *, 2> Returns;
  ValueToValueMapTy vmap;
  // Map params.
  auto entryParamIt = F->arg_begin();
  for (Argument &param : Orig->args()) {
    vmap[&param] = (entryParamIt++);
  }

  llvm::CloneFunctionInto(F, Orig, vmap, /*ModuleLevelChagnes*/ false, Returns);
  TypeSys.CopyFunctionAnnotation(F, Orig, SrcTypeSys);

  return F;
}

// Clone shader entry function to be called by other functions.
// The original function will be used as shader entry.
void CloneShaderEntry(Function *ShaderF, StringRef EntryName, HLModule &HLM) {
  Function *F = CloneFunction(ShaderF, "", HLM.GetModule(), HLM.GetTypeSystem(),
                              HLM.GetTypeSystem());

  F->takeName(ShaderF);
  F->setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
  // Set to name before mangled.
  ShaderF->setName(EntryName);

  DxilFunctionAnnotation *annot = HLM.GetFunctionAnnotation(F);
  DxilParameterAnnotation &cloneRetAnnot = annot->GetRetTypeAnnotation();
  // Clear semantic for cloned one.
  cloneRetAnnot.SetSemanticString("");
  cloneRetAnnot.SetSemanticIndexVec({});
  for (unsigned i = 0; i < annot->GetNumParameters(); i++) {
    DxilParameterAnnotation &cloneParamAnnot = annot->GetParameterAnnotation(i);
    // Clear semantic for cloned one.
    cloneParamAnnot.SetSemanticString("");
    cloneParamAnnot.SetSemanticIndexVec({});
  }
}
} // namespace

namespace {

bool IsPatchConstantFunction(
    const Function *F, StringMap<PatchConstantInfo> &patchConstantFunctionMap) {
  DXASSERT_NOMSG(F != nullptr);
  for (auto &&p : patchConstantFunctionMap) {
    if (p.second.Func == F)
      return true;
  }
  return false;
}

void SetPatchConstantFunctionWithAttr(
    const EntryFunctionInfo &EntryFunc,
    const clang::HLSLPatchConstantFuncAttr *PatchConstantFuncAttr,
    StringMap<PatchConstantInfo> &patchConstantFunctionMap,
    std::unordered_map<Function *, std::unique_ptr<DxilFunctionProps>>
        &patchConstantFunctionPropsMap,
    HLModule &HLM, clang::CodeGen::CodeGenModule &CGM) {
  StringRef funcName = PatchConstantFuncAttr->getFunctionName();

  auto Entry = patchConstantFunctionMap.find(funcName);
  if (Entry == patchConstantFunctionMap.end()) {
    clang::DiagnosticsEngine &Diags = CGM.getDiags();
    unsigned DiagID = Diags.getCustomDiagID(
        clang::DiagnosticsEngine::Error, "Cannot find patchconstantfunc %0.");
    Diags.Report(PatchConstantFuncAttr->getLocation(), DiagID) << funcName;
    return;
  }

  if (Entry->second.NumOverloads != 1) {
    clang::DiagnosticsEngine &Diags = CGM.getDiags();
    unsigned DiagID =
        Diags.getCustomDiagID(clang::DiagnosticsEngine::Warning,
                              "Multiple overloads of patchconstantfunc %0.");
    unsigned NoteID = Diags.getCustomDiagID(clang::DiagnosticsEngine::Note,
                                            "This overload was selected.");
    Diags.Report(PatchConstantFuncAttr->getLocation(), DiagID) << funcName;
    Diags.Report(Entry->second.SL, NoteID);
  }

  Function *patchConstFunc = Entry->second.Func;
  DXASSERT(
      HLM.HasDxilFunctionProps(EntryFunc.Func),
      " else AddHLSLFunctionInfo did not save the dxil function props for the "
      "HS entry.");
  HLM.SetPatchConstantFunctionForHS(EntryFunc.Func, patchConstFunc);
  DXASSERT_NOMSG(patchConstantFunctionPropsMap.count(patchConstFunc));
  // Check no inout parameter for patch constant function.
  DxilFunctionAnnotation *patchConstFuncAnnotation =
      HLM.GetFunctionAnnotation(patchConstFunc);
  for (unsigned i = 0; i < patchConstFuncAnnotation->GetNumParameters(); i++) {
    if (patchConstFuncAnnotation->GetParameterAnnotation(i)
            .GetParamInputQual() == DxilParamInputQual::Inout) {
      clang::DiagnosticsEngine &Diags = CGM.getDiags();
      unsigned DiagID = Diags.getCustomDiagID(
          clang::DiagnosticsEngine::Error,
          "Patch Constant function %0 should not have inout param.");
      Diags.Report(Entry->second.SL, DiagID) << funcName;
    }
  }
}

void SetPatchConstantFunction(
    const EntryFunctionInfo &EntryFunc,
    std::unordered_map<Function *, const clang::HLSLPatchConstantFuncAttr *>
        &HSEntryPatchConstantFuncAttr,
    StringMap<PatchConstantInfo> &patchConstantFunctionMap,
    std::unordered_map<Function *, std::unique_ptr<DxilFunctionProps>>
        &patchConstantFunctionPropsMap,
    HLModule &HLM, clang::CodeGen::CodeGenModule &CGM) {

  auto AttrsIter = HSEntryPatchConstantFuncAttr.find(EntryFunc.Func);

  DXASSERT(AttrsIter != HSEntryPatchConstantFuncAttr.end(),
           "we have checked this in AddHLSLFunctionInfo()");

  SetPatchConstantFunctionWithAttr(EntryFunc, AttrsIter->second,
                                   patchConstantFunctionMap,
                                   patchConstantFunctionPropsMap, HLM, CGM);
}
} // namespace

namespace {

// For case like:
// cbuffer A {
//  float a;
//  int b;
//}
//
// const static struct {
//  float a;
//  int b;
//}  ST = { a, b };
// Replace user of ST with a and b.
bool ReplaceConstStaticGlobalUser(GEPOperator *GEP,
                                  std::vector<Constant *> &InitList,
                                  IRBuilder<> &Builder) {
  if (GEP->getNumIndices() < 2) {
    // Don't use sub element.
    return false;
  }

  SmallVector<Value *, 4> idxList;
  auto iter = GEP->idx_begin();
  idxList.emplace_back(*(iter++));
  ConstantInt *subIdx = dyn_cast<ConstantInt>(*(iter++));

  DXASSERT(subIdx, "else dynamic indexing on struct field");
  unsigned subIdxImm = subIdx->getLimitedValue();
  DXASSERT(subIdxImm < InitList.size(), "else struct index out of bound");

  Constant *subPtr = InitList[subIdxImm];
  // Move every idx to idxList except idx for InitList.
  while (iter != GEP->idx_end()) {
    idxList.emplace_back(*(iter++));
  }
  Value *NewGEP = Builder.CreateGEP(subPtr, idxList);
  GEP->replaceAllUsesWith(NewGEP);
  return true;
}

} // namespace

namespace CGHLSLMSHelper {
void ReplaceConstStaticGlobals(
    std::unordered_map<GlobalVariable *, std::vector<Constant *>>
        &staticConstGlobalInitListMap,
    std::unordered_map<GlobalVariable *, Function *>
        &staticConstGlobalCtorMap) {

  for (auto &iter : staticConstGlobalInitListMap) {
    GlobalVariable *GV = iter.first;
    std::vector<Constant *> &InitList = iter.second;
    LLVMContext &Ctx = GV->getContext();
    // Do the replace.
    bool bPass = true;
    for (User *U : GV->users()) {
      IRBuilder<> Builder(Ctx);
      if (GetElementPtrInst *GEPInst = dyn_cast<GetElementPtrInst>(U)) {
        Builder.SetInsertPoint(GEPInst);
        bPass &= ReplaceConstStaticGlobalUser(cast<GEPOperator>(GEPInst),
                                              InitList, Builder);
      } else if (GEPOperator *GEP = dyn_cast<GEPOperator>(U)) {
        bPass &= ReplaceConstStaticGlobalUser(GEP, InitList, Builder);
      } else {
        DXASSERT(false, "invalid user of const static global");
      }
    }
    // Clear the Ctor which is useless now.
    if (bPass) {
      Function *Ctor = staticConstGlobalCtorMap[GV];
      Ctor->getBasicBlockList().clear();
      BasicBlock *Entry = BasicBlock::Create(Ctx, "", Ctor);
      IRBuilder<> Builder(Entry);
      Builder.CreateRetVoid();
    }
  }
}
} // namespace CGHLSLMSHelper

namespace {

Value *CastLdValue(Value *Ptr, llvm::Type *FromTy, llvm::Type *ToTy,
                   IRBuilder<> &Builder) {
  if (ToTy->isVectorTy()) {
    unsigned vecSize = ToTy->getVectorNumElements();
    if (vecSize == 1 && ToTy->getVectorElementType() == FromTy) {
      Value *V = Builder.CreateLoad(Ptr);
      // ScalarToVec1Splat
      // Change scalar into vec1.
      Value *Vec1 = UndefValue::get(ToTy);
      return Builder.CreateInsertElement(Vec1, V, (uint64_t)0);
    } else if (vecSize == 1 && FromTy->isIntegerTy() &&
               ToTy->getVectorElementType()->isIntegerTy(1)) {
      // load(bitcast i32* to <1 x i1>*)
      // Rewrite to
      // insertelement(icmp ne (load i32*), 0)
      Value *IntV = Builder.CreateLoad(Ptr);
      Value *BoolV = Builder.CreateICmpNE(
          IntV, ConstantInt::get(IntV->getType(), 0), "tobool");
      Value *Vec1 = UndefValue::get(ToTy);
      return Builder.CreateInsertElement(Vec1, BoolV, (uint64_t)0);
    } else if (FromTy->isVectorTy() && vecSize == 1) {
      Value *V = Builder.CreateLoad(Ptr);
      // VectorTrunc
      // Change vector into vec1.
      int mask[] = {0};
      return Builder.CreateShuffleVector(V, V, mask);
    } else if (FromTy->isArrayTy()) {
      llvm::Type *FromEltTy = FromTy->getArrayElementType();

      llvm::Type *ToEltTy = ToTy->getVectorElementType();
      if (FromTy->getArrayNumElements() == vecSize && FromEltTy == ToEltTy) {
        // ArrayToVector.
        Value *NewLd = UndefValue::get(ToTy);
        Value *zeroIdx = Builder.getInt32(0);
        for (unsigned i = 0; i < vecSize; i++) {
          Value *GEP =
              Builder.CreateInBoundsGEP(Ptr, {zeroIdx, Builder.getInt32(i)});
          Value *Elt = Builder.CreateLoad(GEP);
          NewLd = Builder.CreateInsertElement(NewLd, Elt, i);
        }
        return NewLd;
      }
    }
  } else if (FromTy == Builder.getInt1Ty()) {
    Value *V = Builder.CreateLoad(Ptr);
    // BoolCast
    DXASSERT_NOMSG(ToTy->isIntegerTy());
    return Builder.CreateZExt(V, ToTy);
  }

  return nullptr;
}

Value *CastStValue(Value *Ptr, Value *V, llvm::Type *FromTy, llvm::Type *ToTy,
                   IRBuilder<> &Builder) {
  if (ToTy->isVectorTy()) {
    unsigned vecSize = ToTy->getVectorNumElements();
    if (vecSize == 1 && ToTy->getVectorElementType() == FromTy) {
      // ScalarToVec1Splat
      // Change vec1 back to scalar.
      Value *Elt = Builder.CreateExtractElement(V, (uint64_t)0);
      return Elt;
    } else if (FromTy->isVectorTy() && vecSize == 1) {
      // VectorTrunc
      // Change vec1 into vector.
      // Should not happen.
      // Reported error at Sema::ImpCastExprToType.
      DXASSERT_NOMSG(0);
    } else if (FromTy->isArrayTy()) {
      llvm::Type *FromEltTy = FromTy->getArrayElementType();

      llvm::Type *ToEltTy = ToTy->getVectorElementType();
      if (FromTy->getArrayNumElements() == vecSize && FromEltTy == ToEltTy) {
        // ArrayToVector.
        Value *zeroIdx = Builder.getInt32(0);
        for (unsigned i = 0; i < vecSize; i++) {
          Value *Elt = Builder.CreateExtractElement(V, i);
          Value *GEP =
              Builder.CreateInBoundsGEP(Ptr, {zeroIdx, Builder.getInt32(i)});
          Builder.CreateStore(Elt, GEP);
        }
        // The store already done.
        // Return null to ignore use of the return value.
        return nullptr;
      }
    }
  } else if (FromTy == Builder.getInt1Ty()) {
    // BoolCast
    // Change i1 to ToTy.
    DXASSERT_NOMSG(ToTy->isIntegerTy());
    Value *CastV = Builder.CreateICmpNE(V, ConstantInt::get(V->getType(), 0));
    return CastV;
  }

  return nullptr;
}

bool SimplifyBitCastLoad(LoadInst *LI, llvm::Type *FromTy, llvm::Type *ToTy,
                         Value *Ptr) {
  IRBuilder<> Builder(LI);
  // Cast FromLd to ToTy.
  Value *CastV = CastLdValue(Ptr, FromTy, ToTy, Builder);
  if (CastV) {
    LI->replaceAllUsesWith(CastV);
    return true;
  } else {
    return false;
  }
}

bool SimplifyBitCastStore(StoreInst *SI, llvm::Type *FromTy, llvm::Type *ToTy,
                          Value *Ptr) {
  IRBuilder<> Builder(SI);
  Value *V = SI->getValueOperand();
  // Cast Val to FromTy.
  Value *CastV = CastStValue(Ptr, V, FromTy, ToTy, Builder);
  if (CastV) {
    Builder.CreateStore(CastV, Ptr);
    return true;
  } else {
    return false;
  }
}

bool SimplifyBitCastGEP(GEPOperator *GEP, llvm::Type *FromTy, llvm::Type *ToTy,
                        Value *Ptr) {
  if (ToTy->isVectorTy()) {
    unsigned vecSize = ToTy->getVectorNumElements();
    if (vecSize == 1 && ToTy->getVectorElementType() == FromTy) {
      // ScalarToVec1Splat
      GEP->replaceAllUsesWith(Ptr);
      return true;
    } else if (FromTy->isVectorTy() && vecSize == 1) {
      // VectorTrunc
      DXASSERT_NOMSG(
          !isa<llvm::VectorType>(GEP->getType()->getPointerElementType()));
      IRBuilder<> Builder(FromTy->getContext());
      if (Instruction *I = dyn_cast<Instruction>(GEP))
        Builder.SetInsertPoint(I);
      std::vector<Value *> idxList(GEP->idx_begin(), GEP->idx_end());
      Value *NewGEP = Builder.CreateInBoundsGEP(Ptr, idxList);
      GEP->replaceAllUsesWith(NewGEP);
      return true;
    } else if (FromTy->isArrayTy()) {
      llvm::Type *FromEltTy = FromTy->getArrayElementType();

      llvm::Type *ToEltTy = ToTy->getVectorElementType();
      if (FromTy->getArrayNumElements() == vecSize && FromEltTy == ToEltTy) {
        // ArrayToVector.
      }
    }
  } else if (FromTy == llvm::Type::getInt1Ty(FromTy->getContext())) {
    // BoolCast
  }
  return false;
}
typedef SmallPtrSet<Instruction *, 4> SmallInstSet;
void SimplifyBitCast(BitCastOperator *BC, SmallInstSet &deadInsts) {
  Value *Ptr = BC->getOperand(0);
  llvm::Type *FromTy = Ptr->getType();
  llvm::Type *ToTy = BC->getType();

  if (!FromTy->isPointerTy() || !ToTy->isPointerTy())
    return;

  FromTy = FromTy->getPointerElementType();
  ToTy = ToTy->getPointerElementType();

  // Take care case like %2 = bitcast %struct.T* %1 to <1 x float>*.
  bool GEPCreated = false;
  if (FromTy->isStructTy()) {
    IRBuilder<> Builder(FromTy->getContext());
    if (Instruction *I = dyn_cast<Instruction>(BC))
      Builder.SetInsertPoint(I);

    Value *zeroIdx = Builder.getInt32(0);
    unsigned nestLevel = 1;
    while (llvm::StructType *ST = dyn_cast<llvm::StructType>(FromTy)) {
      if (ST->getNumElements() == 0)
        break;
      FromTy = ST->getElementType(0);
      nestLevel++;
    }
    std::vector<Value *> idxList(nestLevel, zeroIdx);
    Ptr = Builder.CreateGEP(Ptr, idxList);
    GEPCreated = true;
  }

  for (User *U : BC->users()) {
    if (LoadInst *LI = dyn_cast<LoadInst>(U)) {
      if (SimplifyBitCastLoad(LI, FromTy, ToTy, Ptr)) {
        LI->dropAllReferences();
        deadInsts.insert(LI);
      }
    } else if (StoreInst *SI = dyn_cast<StoreInst>(U)) {
      if (SimplifyBitCastStore(SI, FromTy, ToTy, Ptr)) {
        SI->dropAllReferences();
        deadInsts.insert(SI);
      }
    } else if (GEPOperator *GEP = dyn_cast<GEPOperator>(U)) {
      if (SimplifyBitCastGEP(GEP, FromTy, ToTy, Ptr))
        if (Instruction *I = dyn_cast<Instruction>(GEP)) {
          I->dropAllReferences();
          deadInsts.insert(I);
        }
    } else if (dyn_cast<CallInst>(U)) {
      // Skip function call.
    } else if (dyn_cast<BitCastInst>(U)) {
      // Skip bitcast.
    } else if (dyn_cast<AddrSpaceCastInst>(U)) {
      // Skip addrspacecast.
    } else {
      DXASSERT(0, "not support yet");
    }
  }

  // We created a GEP instruction but didn't end up consuming it, so delete it.
  if (GEPCreated && Ptr->use_empty()) {
    if (GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(Ptr))
      GEP->eraseFromParent();
    else
      cast<Constant>(Ptr)->destroyConstant();
  }
}

typedef float(__cdecl *FloatUnaryEvalFuncType)(float);
typedef double(__cdecl *DoubleUnaryEvalFuncType)(double);

typedef APInt(__cdecl *IntBinaryEvalFuncType)(const APInt &, const APInt &);
typedef float(__cdecl *FloatBinaryEvalFuncType)(float, float);
typedef double(__cdecl *DoubleBinaryEvalFuncType)(double, double);

typedef APInt(__cdecl *IntTernaryEvalFuncType)(const APInt &, const APInt &,
                                               const APInt &);
typedef float(__cdecl *FloatTernaryEvalFuncType)(float, float, float);
typedef double(__cdecl *DoubleTernaryEvalFuncType)(double, double, double);

Value *EvalUnaryIntrinsic(ConstantFP *fpV, FloatUnaryEvalFuncType floatEvalFunc,
                          DoubleUnaryEvalFuncType doubleEvalFunc) {
  llvm::Type *Ty = fpV->getType();
  Value *Result = nullptr;
  if (Ty->isDoubleTy()) {
    double dV = fpV->getValueAPF().convertToDouble();
    Value *dResult = ConstantFP::get(Ty, doubleEvalFunc(dV));
    Result = dResult;
  } else {
    DXASSERT_NOMSG(Ty->isFloatTy());
    float fV = fpV->getValueAPF().convertToFloat();
    Value *dResult = ConstantFP::get(Ty, floatEvalFunc(fV));
    Result = dResult;
  }
  return Result;
}

Value *EvalBinaryIntrinsic(Constant *cV0, Constant *cV1,
                           FloatBinaryEvalFuncType floatEvalFunc,
                           DoubleBinaryEvalFuncType doubleEvalFunc,
                           IntBinaryEvalFuncType intEvalFunc) {
  llvm::Type *Ty = cV0->getType();
  Value *Result = nullptr;
  if (Ty->isDoubleTy()) {
    ConstantFP *fpV0 = cast<ConstantFP>(cV0);
    ConstantFP *fpV1 = cast<ConstantFP>(cV1);
    double dV0 = fpV0->getValueAPF().convertToDouble();
    double dV1 = fpV1->getValueAPF().convertToDouble();
    Value *dResult = ConstantFP::get(Ty, doubleEvalFunc(dV0, dV1));
    Result = dResult;
  } else if (Ty->isFloatTy()) {
    ConstantFP *fpV0 = cast<ConstantFP>(cV0);
    ConstantFP *fpV1 = cast<ConstantFP>(cV1);
    float fV0 = fpV0->getValueAPF().convertToFloat();
    float fV1 = fpV1->getValueAPF().convertToFloat();
    Value *dResult = ConstantFP::get(Ty, floatEvalFunc(fV0, fV1));
    Result = dResult;
  } else {
    DXASSERT_NOMSG(Ty->isIntegerTy());
    DXASSERT_NOMSG(intEvalFunc);
    ConstantInt *ciV0 = cast<ConstantInt>(cV0);
    ConstantInt *ciV1 = cast<ConstantInt>(cV1);
    const APInt &iV0 = ciV0->getValue();
    const APInt &iV1 = ciV1->getValue();
    Value *dResult = ConstantInt::get(Ty, intEvalFunc(iV0, iV1));
    Result = dResult;
  }
  return Result;
}

Value *EvalTernaryIntrinsic(Constant *cV0, Constant *cV1, Constant *cV2,
                            FloatTernaryEvalFuncType floatEvalFunc,
                            DoubleTernaryEvalFuncType doubleEvalFunc,
                            IntTernaryEvalFuncType intEvalFunc) {
  llvm::Type *Ty = cV0->getType();
  Value *Result = nullptr;
  if (Ty->isDoubleTy()) {
    ConstantFP *fpV0 = cast<ConstantFP>(cV0);
    ConstantFP *fpV1 = cast<ConstantFP>(cV1);
    ConstantFP *fpV2 = cast<ConstantFP>(cV2);
    double dV0 = fpV0->getValueAPF().convertToDouble();
    double dV1 = fpV1->getValueAPF().convertToDouble();
    double dV2 = fpV2->getValueAPF().convertToDouble();
    Value *dResult = ConstantFP::get(Ty, doubleEvalFunc(dV0, dV1, dV2));
    Result = dResult;
  } else if (Ty->isFloatTy()) {
    ConstantFP *fpV0 = cast<ConstantFP>(cV0);
    ConstantFP *fpV1 = cast<ConstantFP>(cV1);
    ConstantFP *fpV2 = cast<ConstantFP>(cV2);
    float fV0 = fpV0->getValueAPF().convertToFloat();
    float fV1 = fpV1->getValueAPF().convertToFloat();
    float fV2 = fpV2->getValueAPF().convertToFloat();
    Value *dResult = ConstantFP::get(Ty, floatEvalFunc(fV0, fV1, fV2));
    Result = dResult;
  } else {
    DXASSERT_NOMSG(Ty->isIntegerTy());
    DXASSERT_NOMSG(intEvalFunc);
    ConstantInt *ciV0 = cast<ConstantInt>(cV0);
    ConstantInt *ciV1 = cast<ConstantInt>(cV1);
    ConstantInt *ciV2 = cast<ConstantInt>(cV2);
    const APInt &iV0 = ciV0->getValue();
    const APInt &iV1 = ciV1->getValue();
    const APInt &iV2 = ciV2->getValue();
    Value *dResult = ConstantInt::get(Ty, intEvalFunc(iV0, iV1, iV2));
    Result = dResult;
  }
  return Result;
}

Value *EvalUnaryIntrinsic(CallInst *CI, FloatUnaryEvalFuncType floatEvalFunc,
                          DoubleUnaryEvalFuncType doubleEvalFunc) {
  Value *V = CI->getArgOperand(0);
  llvm::Type *Ty = CI->getType();
  Value *Result = nullptr;
  if (llvm::VectorType *VT = dyn_cast<llvm::VectorType>(Ty)) {
    Result = UndefValue::get(Ty);
    Constant *CV = cast<Constant>(V);
    IRBuilder<> Builder(CI);
    for (unsigned i = 0; i < VT->getNumElements(); i++) {
      ConstantFP *fpV = cast<ConstantFP>(CV->getAggregateElement(i));
      Value *EltResult = EvalUnaryIntrinsic(fpV, floatEvalFunc, doubleEvalFunc);
      Result = Builder.CreateInsertElement(Result, EltResult, i);
    }
  } else {
    ConstantFP *fpV = cast<ConstantFP>(V);
    Result = EvalUnaryIntrinsic(fpV, floatEvalFunc, doubleEvalFunc);
  }
  CI->replaceAllUsesWith(Result);
  CI->eraseFromParent();
  return Result;
}

Value *EvalBinaryIntrinsic(CallInst *CI, FloatBinaryEvalFuncType floatEvalFunc,
                           DoubleBinaryEvalFuncType doubleEvalFunc,
                           IntBinaryEvalFuncType intEvalFunc = nullptr) {
  Value *V0 = CI->getArgOperand(0);
  Value *V1 = CI->getArgOperand(1);
  llvm::Type *Ty = CI->getType();
  Value *Result = nullptr;
  if (llvm::VectorType *VT = dyn_cast<llvm::VectorType>(Ty)) {
    Result = UndefValue::get(Ty);
    Constant *CV0 = cast<Constant>(V0);
    Constant *CV1 = cast<Constant>(V1);
    IRBuilder<> Builder(CI);
    for (unsigned i = 0; i < VT->getNumElements(); i++) {
      Constant *cV0 = cast<Constant>(CV0->getAggregateElement(i));
      Constant *cV1 = cast<Constant>(CV1->getAggregateElement(i));
      Value *EltResult = EvalBinaryIntrinsic(cV0, cV1, floatEvalFunc,
                                             doubleEvalFunc, intEvalFunc);
      Result = Builder.CreateInsertElement(Result, EltResult, i);
    }
  } else {
    Constant *cV0 = cast<Constant>(V0);
    Constant *cV1 = cast<Constant>(V1);
    Result = EvalBinaryIntrinsic(cV0, cV1, floatEvalFunc, doubleEvalFunc,
                                 intEvalFunc);
  }
  CI->replaceAllUsesWith(Result);
  CI->eraseFromParent();
  return Result;

  CI->eraseFromParent();
  return Result;
}

Value *EvalTernaryIntrinsic(CallInst *CI,
                            FloatTernaryEvalFuncType floatEvalFunc,
                            DoubleTernaryEvalFuncType doubleEvalFunc,
                            IntTernaryEvalFuncType intEvalFunc = nullptr) {
  Value *V0 = CI->getArgOperand(0);
  Value *V1 = CI->getArgOperand(1);
  Value *V2 = CI->getArgOperand(2);
  llvm::Type *Ty = CI->getType();
  Value *Result = nullptr;
  if (llvm::VectorType *VT = dyn_cast<llvm::VectorType>(Ty)) {
    Result = UndefValue::get(Ty);
    Constant *CV0 = cast<Constant>(V0);
    Constant *CV1 = cast<Constant>(V1);
    Constant *CV2 = cast<Constant>(V2);
    IRBuilder<> Builder(CI);
    for (unsigned i = 0; i < VT->getNumElements(); i++) {
      Constant *cV0 = cast<Constant>(CV0->getAggregateElement(i));
      Constant *cV1 = cast<Constant>(CV1->getAggregateElement(i));
      Constant *cV2 = cast<Constant>(CV2->getAggregateElement(i));
      Value *EltResult = EvalTernaryIntrinsic(cV0, cV1, cV2, floatEvalFunc,
                                              doubleEvalFunc, intEvalFunc);
      Result = Builder.CreateInsertElement(Result, EltResult, i);
    }
  } else {
    Constant *cV0 = cast<Constant>(V0);
    Constant *cV1 = cast<Constant>(V1);
    Constant *cV2 = cast<Constant>(V2);
    Result = EvalTernaryIntrinsic(cV0, cV1, cV2, floatEvalFunc, doubleEvalFunc,
                                  intEvalFunc);
  }
  CI->replaceAllUsesWith(Result);
  CI->eraseFromParent();
  return Result;

  CI->eraseFromParent();
  return Result;
}

void SimpleTransformForHLDXIRInst(Instruction *I, SmallInstSet &deadInsts) {

  unsigned opcode = I->getOpcode();
  switch (opcode) {
  case Instruction::BitCast: {
    BitCastOperator *BCI = cast<BitCastOperator>(I);
    SimplifyBitCast(BCI, deadInsts);
  } break;
  case Instruction::Load: {
    LoadInst *ldInst = cast<LoadInst>(I);
    DXASSERT(!HLMatrixType::isa(ldInst->getType()),
             "matrix load should use HL LdStMatrix");
    Value *Ptr = ldInst->getPointerOperand();
    if (ConstantExpr *CE = dyn_cast_or_null<ConstantExpr>(Ptr)) {
      if (BitCastOperator *BCO = dyn_cast<BitCastOperator>(CE)) {
        SimplifyBitCast(BCO, deadInsts);
      }
    }
  } break;
  case Instruction::Store: {
    StoreInst *stInst = cast<StoreInst>(I);
    Value *V = stInst->getValueOperand();
    DXASSERT_LOCALVAR(V, !HLMatrixType::isa(V->getType()),
                      "matrix store should use HL LdStMatrix");
    Value *Ptr = stInst->getPointerOperand();
    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(Ptr)) {
      if (BitCastOperator *BCO = dyn_cast<BitCastOperator>(CE)) {
        SimplifyBitCast(BCO, deadInsts);
      }
    }
  } break;
  }
}

} // namespace

namespace CGHLSLMSHelper {

Value *TryEvalIntrinsic(CallInst *CI, IntrinsicOp intriOp,
                        hlsl::LangStd hlslVersion) {
  switch (intriOp) {
  case IntrinsicOp::IOP_tan: {
    return EvalUnaryIntrinsic(CI, tanf, tan);
  } break;
  case IntrinsicOp::IOP_tanh: {
    return EvalUnaryIntrinsic(CI, tanhf, tanh);
  } break;
  case IntrinsicOp::IOP_sin: {
    return EvalUnaryIntrinsic(CI, sinf, sin);
  } break;
  case IntrinsicOp::IOP_sinh: {
    return EvalUnaryIntrinsic(CI, sinhf, sinh);
  } break;
  case IntrinsicOp::IOP_cos: {
    return EvalUnaryIntrinsic(CI, cosf, cos);
  } break;
  case IntrinsicOp::IOP_cosh: {
    return EvalUnaryIntrinsic(CI, coshf, cosh);
  } break;
  case IntrinsicOp::IOP_asin: {
    return EvalUnaryIntrinsic(CI, asinf, asin);
  } break;
  case IntrinsicOp::IOP_acos: {
    return EvalUnaryIntrinsic(CI, acosf, acos);
  } break;
  case IntrinsicOp::IOP_atan: {
    return EvalUnaryIntrinsic(CI, atanf, atan);
  } break;
  case IntrinsicOp::IOP_atan2: {
    Value *atanV = Atan2(CI);
    CI->replaceAllUsesWith(atanV);
    CI->eraseFromParent();
    return atanV;
  } break;
  case IntrinsicOp::IOP_sqrt: {
    return EvalUnaryIntrinsic(CI, sqrtf, sqrt);
  } break;
  case IntrinsicOp::IOP_rsqrt: {
    auto rsqrtF = [](float v) -> float { return 1.0 / sqrtf(v); };
    auto rsqrtD = [](double v) -> double { return 1.0 / sqrt(v); };

    return EvalUnaryIntrinsic(CI, rsqrtF, rsqrtD);
  } break;
  case IntrinsicOp::IOP_exp: {
    return EvalUnaryIntrinsic(CI, expf, exp);
  } break;
  case IntrinsicOp::IOP_exp2: {
    return EvalUnaryIntrinsic(CI, exp2f, exp2);
  } break;
  case IntrinsicOp::IOP_log: {
    return EvalUnaryIntrinsic(CI, logf, log);
  } break;
  case IntrinsicOp::IOP_log10: {
    return EvalUnaryIntrinsic(CI, log10f, log10);
  } break;
  case IntrinsicOp::IOP_log2: {
    return EvalUnaryIntrinsic(CI, log2f, log2);
  } break;
  case IntrinsicOp::IOP_pow: {
    return EvalBinaryIntrinsic(CI, powf, pow);
  } break;
  case IntrinsicOp::IOP_max: {
    auto maxF = [](float a, float b) -> float { return a > b ? a : b; };
    auto maxD = [](double a, double b) -> double { return a > b ? a : b; };
    auto imaxI = [](const APInt &a, const APInt &b) -> APInt {
      return a.sgt(b) ? a : b;
    };
    return EvalBinaryIntrinsic(CI, maxF, maxD, imaxI);
  } break;
  case IntrinsicOp::IOP_min: {
    auto minF = [](float a, float b) -> float { return a < b ? a : b; };
    auto minD = [](double a, double b) -> double { return a < b ? a : b; };
    auto iminI = [](const APInt &a, const APInt &b) -> APInt {
      return a.slt(b) ? a : b;
    };
    return EvalBinaryIntrinsic(CI, minF, minD, iminI);
  } break;
  case IntrinsicOp::IOP_umax: {
    DXASSERT_NOMSG(
        CI->getArgOperand(0)->getType()->getScalarType()->isIntegerTy());
    auto umaxI = [](const APInt &a, const APInt &b) -> APInt {
      return a.ugt(b) ? a : b;
    };
    return EvalBinaryIntrinsic(CI, nullptr, nullptr, umaxI);
  } break;
  case IntrinsicOp::IOP_umin: {
    DXASSERT_NOMSG(
        CI->getArgOperand(0)->getType()->getScalarType()->isIntegerTy());
    auto uminI = [](const APInt &a, const APInt &b) -> APInt {
      return a.ult(b) ? a : b;
    };
    return EvalBinaryIntrinsic(CI, nullptr, nullptr, uminI);
  } break;
  case IntrinsicOp::IOP_rcp: {
    auto rcpF = [](float v) -> float { return 1.0 / v; };
    auto rcpD = [](double v) -> double { return 1.0 / v; };

    return EvalUnaryIntrinsic(CI, rcpF, rcpD);
  } break;
  case IntrinsicOp::IOP_ceil: {
    return EvalUnaryIntrinsic(CI, ceilf, ceil);
  } break;
  case IntrinsicOp::IOP_floor: {
    return EvalUnaryIntrinsic(CI, floorf, floor);
  } break;
  case IntrinsicOp::IOP_round: {
    // round intrinsic could exhibit different behaviour for constant and
    // runtime evaluations. E.g., for round(0.5): constant evaluation results in
    // 1 (away from zero rounding), while runtime evaluation results in 0
    // (nearest even rounding).
    //
    // For back compat, DXC still preserves the above behavior for language
    // versions 2016 or below. However, for newer language versions, DXC now
    // always use nearest even for round() intrinsic in all cases.
    if (hlslVersion <= hlsl::LangStd::v2016) {
      return EvalUnaryIntrinsic(CI, roundf, round);
    } else {
      auto roundingMode = fegetround();
      fesetround(FE_TONEAREST);
      Value *result = EvalUnaryIntrinsic(CI, nearbyintf, nearbyint);
      fesetround(roundingMode);
      return result;
    }
  } break;
  case IntrinsicOp::IOP_trunc: {
    return EvalUnaryIntrinsic(CI, truncf, trunc);
  } break;
  case IntrinsicOp::IOP_frac: {
    auto fracF = [](float v) -> float { return v - floor(v); };
    auto fracD = [](double v) -> double { return v - floor(v); };

    return EvalUnaryIntrinsic(CI, fracF, fracD);
  } break;
  case IntrinsicOp::IOP_isnan: {
    Constant *cNan = IsNaN(CI);
    CI->replaceAllUsesWith(cNan);
    CI->eraseFromParent();
    return cNan;
  } break;
  case IntrinsicOp::IOP_clamp: {
    auto clampF = [](float a, float b, float c) {
      return a < b ? b : a > c ? c : a;
    };
    auto clampD = [](double a, double b, double c) {
      return a < b ? b : a > c ? c : a;
    };
    auto clampI = [](const APInt &a, const APInt &b, const APInt &c) -> APInt {
      return a.slt(b) ? b : a.sgt(c) ? c : a;
    };
    return EvalTernaryIntrinsic(CI, clampF, clampD, clampI);
  } break;
  default:
    return nullptr;
  }
}

// Do simple transform to make later lower pass easier.
void SimpleTransformForHLDXIR(llvm::Module *pM) {
  SmallInstSet deadInsts;
  for (Function &F : pM->functions()) {
    for (BasicBlock &BB : F.getBasicBlockList()) {
      for (BasicBlock::iterator Iter = BB.begin(); Iter != BB.end();) {
        Instruction *I = (Iter++);
        if (deadInsts.count(I))
          continue; // Skip dead instructions
        SimpleTransformForHLDXIRInst(I, deadInsts);
      }
    }
  }

  for (Instruction *I : deadInsts)
    I->dropAllReferences();
  for (Instruction *I : deadInsts)
    I->eraseFromParent();
  deadInsts.clear();

  for (GlobalVariable &GV : pM->globals()) {
    if (dxilutil::IsStaticGlobal(&GV)) {
      for (User *U : GV.users()) {
        if (BitCastOperator *BCO = dyn_cast<BitCastOperator>(U)) {
          SimplifyBitCast(BCO, deadInsts);
        }
      }
    }
  }

  for (Instruction *I : deadInsts)
    I->dropAllReferences();
  for (Instruction *I : deadInsts)
    I->eraseFromParent();
}
} // namespace CGHLSLMSHelper

namespace {

unsigned RoundToAlign(unsigned num, unsigned mod) {
  // round num to next highest mod
  if (mod != 0)
    return mod * ((num + mod - 1) / mod);
  return num;
}

// Retrieve the last scalar or vector element type.
// This has to be recursive for the nasty empty struct case.
// returns true if found, false if we must backtrack.
bool RetrieveLastElementType(Type *Ty, Type *&EltTy) {
  if (Ty->isStructTy()) {
    if (Ty->getStructNumElements() == 0)
      return false;
    for (unsigned i = Ty->getStructNumElements(); i > 0; --i) {
      if (RetrieveLastElementType(Ty->getStructElementType(i - 1), EltTy))
        return true;
    }
  } else if (Ty->isArrayTy()) {
    if (RetrieveLastElementType(Ty->getArrayElementType(), EltTy))
      return true;
  } else if ((Ty->isVectorTy() || Ty->isSingleValueType())) {
    EltTy = Ty->getScalarType();
    return true;
  }
  return false;
}

// Here the size is CB size.
// Offset still needs to be aligned based on type since this
// is the legacy cbuffer global path.
unsigned AlignCBufferOffset(unsigned offset, unsigned size, llvm::Type *Ty,
                            bool bRowMajor, bool bMinPrecMode,
                            bool &bCurRowIsMinPrec) {
  DXASSERT(!(offset & 1), "otherwise we have an invalid offset.");
  // resources, empty structure, or structures with only resources have
  // zero size, and need no alignment.
  if (size == 0)
    return offset;
  bool bNeedNewRow = Ty->isArrayTy();
  // In min-precision mode, a new row is needed when
  // going into or out of min-precision component type.
  if (!bNeedNewRow) {
    bool bMinPrec = false;
    if (Ty->isStructTy()) {
      if (HLMatrixType mat = HLMatrixType::dyn_cast(Ty)) {
        bNeedNewRow |= !bRowMajor && mat.getNumColumns() > 1;
        bNeedNewRow |= bRowMajor && mat.getNumRows() > 1;
        bMinPrec = bMinPrecMode &&
                   mat.getElementType(false)->getScalarSizeInBits() < 32;
      } else {
        bNeedNewRow = true;
        if (bMinPrecMode) {
          // Need to get min-prec of last element of structure,
          // in case we pack something else into the end.
          Type *EltTy = nullptr;
          if (RetrieveLastElementType(Ty, EltTy))
            bCurRowIsMinPrec = EltTy->getScalarSizeInBits() < 32;
        }
      }
    } else {
      DXASSERT_NOMSG(Ty->isVectorTy() || Ty->isSingleValueType());
      // vector or scalar
      bMinPrec = bMinPrecMode && Ty->getScalarSizeInBits() < 32;
    }
    if (bMinPrecMode) {
      bNeedNewRow |= bCurRowIsMinPrec != bMinPrec;
      bCurRowIsMinPrec = bMinPrec;
    }
  }
  unsigned scalarSizeInBytes = Ty->getScalarSizeInBits() / 8;

  return AlignBufferOffsetInLegacy(offset, size, scalarSizeInBytes,
                                   bNeedNewRow);
}

unsigned AllocateDxilConstantBuffer(
    HLCBuffer &CB,
    std::unordered_map<Constant *, DxilFieldAnnotation> &constVarAnnotationMap,
    bool bMinPrecMode) {
  unsigned offset = 0;

  // Scan user allocated constants first.
  // Update offset.
  for (const std::unique_ptr<DxilResourceBase> &C : CB.GetConstants()) {
    if (C->GetLowerBound() == UINT_MAX)
      continue;
    unsigned size = C->GetRangeSize();
    unsigned nextOffset = size + C->GetLowerBound();
    if (offset < nextOffset)
      offset = nextOffset;
  }

  // Alloc after user allocated constants.
  bool bCurRowIsMinPrec = false;
  for (const std::unique_ptr<DxilResourceBase> &C : CB.GetConstants()) {
    if (C->GetLowerBound() != UINT_MAX)
      continue;

    unsigned size = C->GetRangeSize();
    llvm::Type *Ty = C->GetHLSLType()->getPointerElementType();
    auto fieldAnnotation = constVarAnnotationMap.at(C->GetGlobalSymbol());
    bool bRowMajor = HLMatrixType::isa(Ty)
                         ? fieldAnnotation.GetMatrixAnnotation().Orientation ==
                               MatrixOrientation::RowMajor
                         : false;
    // Align offset.
    offset = AlignCBufferOffset(offset, size, Ty, bRowMajor, bMinPrecMode,
                                bCurRowIsMinPrec);
    if (C->GetLowerBound() == UINT_MAX) {
      C->SetLowerBound(offset);
    }
    offset += size;
  }
  return offset;
}

void AllocateDxilConstantBuffers(
    HLModule &HLM, std::unordered_map<Constant *, DxilFieldAnnotation>
                       &constVarAnnotationMap) {
  for (unsigned i = 0; i < HLM.GetCBuffers().size(); i++) {
    HLCBuffer &CB = *static_cast<HLCBuffer *>(&(HLM.GetCBuffer(i)));
    unsigned size = AllocateDxilConstantBuffer(
        CB, constVarAnnotationMap, HLM.GetHLOptions().bUseMinPrecision);
    CB.SetSize(size);
  }
}

} // namespace

namespace {

// Instruction pointer wrapper that cleans up the instruction if unused
template <typename _InstructionT> struct CleanupIfUnused {
  _InstructionT *I = nullptr;
  CleanupIfUnused(Value *V) {
    if (V)
      I = cast<_InstructionT>(V);
  }
  ~CleanupIfUnused() {
    if (I && I->user_empty())
      I->eraseFromParent();
    I = nullptr;
  }
  operator _InstructionT *() { return I; }
  _InstructionT *operator->() { return I; }
  operator bool() { return I != nullptr; }
};

// When replacing CBV use, it's possible for the resource itself to be used,
// in which case we need to either use a cast from handle back to the resource
// type, or if it's a pointer, use an alloca where that casted resource is
// stored.
// Later we will mutate the original resource type to a handle type,
// and eliminate this cast.
// This helper produces the cast and a store to new alloca in case either a
// value or a pointer to the original resource type is needed, and cleans up any
// unused instructions afterward.
// Instructions are only produced if hlResTy is non-null, indicating
// ConstantBuffer<> object type rather than legacy cbuffer type that has no
// resource object.
struct HandleToResHelper {
  HandleToResHelper(IRBuilder<> &Builder, Type *hlResTy, Value *Handle,
                    Function &F, HLModule &HLM) {
    if (hlResTy) {
      Cast = cast<CallInst>(
          HLM.EmitHLOperationCall(Builder, HLOpcodeGroup::HLCast,
                                  (unsigned)HLCastOpcode::HandleToResCast,
                                  hlResTy, {Handle}, *HLM.GetModule()));
      IRBuilder<> AllocaBuilder(F.getEntryBlock().getFirstInsertionPt());
      Alloca = AllocaBuilder.CreateAlloca(hlResTy);
      Store = Builder.CreateStore(Cast, Alloca);
    }
  }
  ~HandleToResHelper() {
    if (Alloca && Alloca->hasOneUse()) {
      Store->eraseFromParent();
      Alloca->eraseFromParent();
      if (Cast->user_empty())
        Cast->eraseFromParent();
    }
  }
  operator bool() { return Store != nullptr; }

  StoreInst *Store = nullptr;
  AllocaInst *Alloca = nullptr;
  CallInst *Cast = nullptr;
};

void ReplaceUseInFunction(Value *V, Value *NewV, Function *F,
                          IRBuilder<> &Builder,
                          HandleToResHelper *pH2ResHelper = nullptr) {
  Type *VTy = V->getType()->getPointerElementType();
  bool bIsResourceTy = dxilutil::IsHLSLResourceType(VTy);
  for (auto U = V->user_begin(); U != V->user_end();) {
    User *user = *(U++);
    if (Instruction *I = dyn_cast<Instruction>(user)) {
      if (I->getParent()->getParent() == F) {
        // replace use with GEP if in F
        if (BitCastInst *BCI = dyn_cast<BitCastInst>(I)) {
          if (BCI->getType() == NewV->getType()) {
            I->replaceAllUsesWith(NewV);
            I->eraseFromParent();
            continue;
          }
        }
        // Only replace when type matches.
        if (V->getType() == NewV->getType()) {
          I->replaceUsesOfWith(V, NewV);
          continue;
        }
        if (pH2ResHelper && *pH2ResHelper && bIsResourceTy) {
          // This means CBV case, so use casted value or pointer instead.
          if (LoadInst *LI = dyn_cast<LoadInst>(I)) {
            Instruction *OrigPtrInst =
                dyn_cast<Instruction>(LI->getPointerOperand());
            LI->replaceAllUsesWith(pH2ResHelper->Cast);
            LI->eraseFromParent();
            // Clean up original pointer instruction (GEP) if unused.
            if (OrigPtrInst && OrigPtrInst->user_empty())
              OrigPtrInst->eraseFromParent();
            continue;
          } else {
            I->replaceUsesOfWith(V, pH2ResHelper->Alloca);
            continue;
          }
        }
        DXASSERT(
            false,
            "otherwise, attempting CB user replacement with mismatching type");
      }
    } else {
      // For constant operator, create local clone which use GEP.
      // Only support GEP and bitcast.
      if (GEPOperator *GEPOp = dyn_cast<GEPOperator>(user)) {
        std::vector<Value *> idxList(GEPOp->idx_begin(), GEPOp->idx_end());
        Value *NewGEP = Builder.CreateInBoundsGEP(NewV, idxList);
        ReplaceUseInFunction(GEPOp, NewGEP, F, Builder);
      } else if (GlobalVariable *GV = dyn_cast<GlobalVariable>(user)) {
        // Change the init val into NewV with Store.

        // FIXME: This store needs to be placed in a new init function for
        // this GV, rather than being placed in whichever function we happen
        // to be iterating at this point.  Otherwise it may not actually be
        // initialized when used from another function.
        GV->setInitializer(nullptr);
        Builder.CreateStore(NewV, GV);
      } else {
        // Must be bitcast here.
        BitCastOperator *BC = cast<BitCastOperator>(user);
        Value *NewBC = Builder.CreateBitCast(NewV, BC->getType());
        ReplaceUseInFunction(BC, NewBC, F, Builder);
      }
    }
  }
}

void MarkUsedFunctionForConst(Value *V,
                              std::unordered_set<Function *> &usedFunc) {
  for (auto U = V->user_begin(); U != V->user_end();) {
    User *user = *(U++);
    if (Instruction *I = dyn_cast<Instruction>(user)) {
      Function *F = I->getParent()->getParent();
      usedFunc.insert(F);
    } else {
      // For constant operator, create local clone which use GEP.
      // Only support GEP and bitcast.
      if (GEPOperator *GEPOp = dyn_cast<GEPOperator>(user)) {
        MarkUsedFunctionForConst(GEPOp, usedFunc);
      } else if (GlobalVariable *GV = dyn_cast<GlobalVariable>(user)) {
        MarkUsedFunctionForConst(GV, usedFunc);
      } else {
        // Must be bitcast here.
        BitCastOperator *BC = cast<BitCastOperator>(user);
        MarkUsedFunctionForConst(BC, usedFunc);
      }
    }
  }
}

bool CreateCBufferVariable(HLCBuffer &CB, HLModule &HLM, llvm::Type *HandleTy) {
  bool bUsed = false;
  // Build Struct for CBuffer.
  SmallVector<llvm::Type *, 4> Elements;
  for (const std::unique_ptr<DxilResourceBase> &C : CB.GetConstants()) {
    Value *GV = C->GetGlobalSymbol();
    if (!GV->use_empty())
      bUsed = true;
    // Global variable must be pointer type.
    llvm::Type *Ty = C->GetHLSLType()->getPointerElementType();
    Elements.emplace_back(Ty);
  }
  // Don't create CBuffer variable for unused cbuffer.
  if (!bUsed)
    return false;

  llvm::Module &M = *HLM.GetModule();

  bool isCBArray = CB.IsArray();
  llvm::GlobalVariable *cbGV = nullptr;
  llvm::Type *cbTy = nullptr;

  unsigned cbIndexDepth = 0;
  if (!isCBArray) {
    if (CB.IsView()) {
      llvm::StructType *CBStructTy =
          llvm::StructType::create(CB.GetResultType(), CB.GetGlobalName());
      cbGV = new llvm::GlobalVariable(M, CBStructTy,
                                      /*IsConstant*/ true,
                                      llvm::GlobalValue::ExternalLinkage,
                                      /*InitVal*/ nullptr, CB.GetGlobalName());
      cbTy = cbGV->getType();
    } else {
      llvm::StructType *CBStructTy =
          llvm::StructType::create(Elements, CB.GetGlobalName());
      cbGV = new llvm::GlobalVariable(M, CBStructTy, /*IsConstant*/ true,
                                      llvm::GlobalValue::ExternalLinkage,
                                      /*InitVal*/ nullptr, CB.GetGlobalName());
      cbTy = cbGV->getType();
    }
  } else {
    // For array of ConstantBuffer, create array of struct instead of struct of
    // array.
    DXASSERT(CB.GetConstants().size() == 1,
             "ConstantBuffer should have 1 constant");
    llvm::Type *CBEltTy = CB.GetConstants()[0]
                              ->GetHLSLType()
                              ->getPointerElementType()
                              ->getArrayElementType();
    cbIndexDepth = 1;
    while (CBEltTy->isArrayTy()) {
      CBEltTy = CBEltTy->getArrayElementType();
      cbIndexDepth++;
    }

    // Add one level struct type to match normal case.
    llvm::StructType *CBStructTy =
        llvm::StructType::create({CB.GetResultType()}, CB.GetGlobalName());

    llvm::ArrayType *CBArrayTy =
        llvm::ArrayType::get(CBStructTy, CB.GetRangeSize());
    cbGV = new llvm::GlobalVariable(M, CBArrayTy, /*IsConstant*/ true,
                                    llvm::GlobalValue::ExternalLinkage,
                                    /*InitVal*/ nullptr, CB.GetGlobalName());

    cbTy = llvm::PointerType::get(CBStructTy,
                                  cbGV->getType()->getPointerAddressSpace());
  }

  CB.SetGlobalSymbol(cbGV);

  llvm::Type *opcodeTy = llvm::Type::getInt32Ty(M.getContext());
  llvm::Type *idxTy = opcodeTy;
  Constant *zeroIdx = ConstantInt::get(opcodeTy, 0);

  Value *HandleArgs[] = {cbGV, zeroIdx};

  llvm::FunctionType *SubscriptFuncTy =
      llvm::FunctionType::get(cbTy, {opcodeTy, HandleTy, idxTy}, false);

  Function *subscriptFunc =
      GetOrCreateHLFunction(M, SubscriptFuncTy, HLOpcodeGroup::HLSubscript,
                            (unsigned)HLSubscriptOpcode::CBufferSubscript);
  Constant *opArg =
      ConstantInt::get(opcodeTy, (unsigned)HLSubscriptOpcode::CBufferSubscript);
  Value *args[] = {opArg, nullptr, zeroIdx};

  llvm::LLVMContext &Context = M.getContext();
  llvm::Type *i32Ty = llvm::Type::getInt32Ty(Context);
  Value *zero = ConstantInt::get(i32Ty, (uint64_t)0);

  std::vector<Value *> indexArray(CB.GetConstants().size());
  std::vector<std::unordered_set<Function *>> constUsedFuncList(
      CB.GetConstants().size());

  for (const std::unique_ptr<DxilResourceBase> &C : CB.GetConstants()) {
    Value *idx = ConstantInt::get(i32Ty, C->GetID());
    indexArray[C->GetID()] = idx;

    Value *GV = C->GetGlobalSymbol();
    MarkUsedFunctionForConst(GV, constUsedFuncList[C->GetID()]);
  }

  // If ConstantBuffer<> style CBV, get resource type used for translating
  // handles back to resource objects.
  Type *hlResTy = nullptr;
  if (CB.IsView()) {
    hlResTy = CB.GetConstants().front()->GetHLSLType()->getPointerElementType();
    hlResTy = dxilutil::StripArrayTypes(hlResTy);
  }

  for (Function &F : M.functions()) {
    if (F.isDeclaration())
      continue;

    if (GetHLOpcodeGroupByName(&F) != HLOpcodeGroup::NotHL)
      continue;

    IRBuilder<> Builder(F.getEntryBlock().getFirstInsertionPt());

    // create HL subscript to make all the use of cbuffer start from it.
    HandleArgs[HLOperandIndex::kCreateHandleResourceOpIdx - 1] = cbGV;
    CleanupIfUnused<CallInst> OrigHandle = HLM.EmitHLOperationCall(
        Builder, HLOpcodeGroup::HLCreateHandle, 0, HandleTy, HandleArgs, M);
    DxilResourceProperties RP = resource_helper::loadPropsFromResourceBase(&CB);
    CleanupIfUnused<CallInst> Handle = CreateAnnotateHandle(
        HLM, OrigHandle, RP, cbGV->getType()->getElementType(), Builder);

    args[HLOperandIndex::kSubscriptObjectOpIdx] = Handle;
    CleanupIfUnused<Instruction> cbSubscript =
        Builder.CreateCall(subscriptFunc, {args});

    // Replace constant var with GEP pGV
    for (const std::unique_ptr<DxilResourceBase> &C : CB.GetConstants()) {
      Value *GV = C->GetGlobalSymbol();
      if (constUsedFuncList[C->GetID()].count(&F) == 0)
        continue;

      Value *idx = indexArray[C->GetID()];
      if (!isCBArray) {
        CleanupIfUnused<Instruction> GEP =
            Builder.CreateInBoundsGEP(cbSubscript, {zero, idx});
        // TODO: make sure the debug info is synced to GEP.
        // GEP->setDebugLoc(GV);

        HandleToResHelper H2ResHelper(Builder, hlResTy, Handle, F, HLM);
        ReplaceUseInFunction(GV, GEP, &F, Builder, &H2ResHelper);

      } else {
        for (auto U = GV->user_begin(); U != GV->user_end();) {
          User *user = *(U++);
          if (user->user_empty())
            continue;
          Instruction *I = dyn_cast<Instruction>(user);
          if (I && I->getParent()->getParent() != &F)
            continue;

          IRBuilder<> *instBuilder = &Builder;
          std::unique_ptr<IRBuilder<>> B;
          if (I) {
            B = llvm::make_unique<IRBuilder<>>(I);
            instBuilder = B.get();
          }

          GEPOperator *GEPOp = cast<GEPOperator>(user);
          std::vector<Value *> idxList;

          DXASSERT(GEPOp->getNumIndices() >= 1 + cbIndexDepth,
                   "must indexing ConstantBuffer array");
          idxList.reserve(GEPOp->getNumIndices() - (cbIndexDepth - 1));

          gep_type_iterator GI = gep_type_begin(*GEPOp),
                            E = gep_type_end(*GEPOp);
          idxList.push_back(GI.getOperand());
          // change array index with 0 for struct index.
          idxList.push_back(zero);
          GI++;
          Value *arrayIdx = GI.getOperand();
          GI++;
          for (unsigned curIndex = 1; GI != E && curIndex < cbIndexDepth;
               ++GI, ++curIndex) {
            arrayIdx = instBuilder->CreateMul(
                arrayIdx, Builder.getInt32(GI->getArrayNumElements()));
            arrayIdx = instBuilder->CreateAdd(arrayIdx, GI.getOperand());
          }

          for (; GI != E; ++GI) {
            idxList.push_back(GI.getOperand());
          }

          HandleArgs[HLOperandIndex::kCreateHandleIndexOpIdx - 1] = arrayIdx;
          CleanupIfUnused<CallInst> OrigHandle = HLM.EmitHLOperationCall(
              *instBuilder, HLOpcodeGroup::HLCreateHandle, 0, HandleTy,
              HandleArgs, M);

          DxilResourceProperties RP =
              resource_helper::loadPropsFromResourceBase(&CB);
          CleanupIfUnused<CallInst> Handle = CreateAnnotateHandle(
              HLM, OrigHandle, RP, cbGV->getType()->getElementType(),
              *instBuilder);

          args[HLOperandIndex::kSubscriptObjectOpIdx] = Handle;
          args[HLOperandIndex::kSubscriptIndexOpIdx] = arrayIdx;

          CleanupIfUnused<Instruction> cbSubscript =
              instBuilder->CreateCall(subscriptFunc, {args});
          CleanupIfUnused<Instruction> NewGEP =
              instBuilder->CreateInBoundsGEP(cbSubscript, idxList);

          HandleToResHelper H2ResHelper(*instBuilder, hlResTy, Handle, F, HLM);
          ReplaceUseInFunction(GEPOp, NewGEP, &F, *instBuilder, &H2ResHelper);
        }
      }
    }
    if (!cbSubscript->user_empty()) {
      // merge GEP use for cbSubscript.
      dxilutil::MergeGepUse(cbSubscript);
    }
  }
  return true;
}

void ConstructCBufferAnnotation(
    HLCBuffer &CB, DxilTypeSystem &dxilTypeSys,
    std::unordered_map<Constant *, DxilFieldAnnotation> &AnnotationMap) {
  Value *GV = CB.GetGlobalSymbol();

  llvm::StructType *CBStructTy =
      dyn_cast<llvm::StructType>(GV->getType()->getPointerElementType());

  if (!CBStructTy) {
    // For Array of ConstantBuffer.
    llvm::ArrayType *CBArrayTy =
        cast<llvm::ArrayType>(GV->getType()->getPointerElementType());
    CBStructTy = cast<llvm::StructType>(CBArrayTy->getArrayElementType());
  }

  DxilStructAnnotation *CBAnnotation =
      dxilTypeSys.AddStructAnnotation(CBStructTy);
  CBAnnotation->SetCBufferSize(CB.GetSize());

  // Set fieldAnnotation for each constant var.
  for (const std::unique_ptr<DxilResourceBase> &C : CB.GetConstants()) {
    Constant *GV = C->GetGlobalSymbol();
    DxilFieldAnnotation &fieldAnnotation =
        CBAnnotation->GetFieldAnnotation(C->GetID());
    fieldAnnotation = AnnotationMap[GV];
    // This is after CBuffer allocation.
    fieldAnnotation.SetCBufferOffset(C->GetLowerBound());
    fieldAnnotation.SetFieldName(C->GetGlobalName());
  }
}

void ConstructCBuffer(
    HLModule &HLM, llvm::Type *CBufferType,
    std::unordered_map<Constant *, DxilFieldAnnotation> &AnnotationMap) {
  DxilTypeSystem &dxilTypeSys = HLM.GetTypeSystem();
  llvm::Type *HandleTy = HLM.GetOP()->GetHandleType();
  for (unsigned i = 0; i < HLM.GetCBuffers().size(); i++) {
    HLCBuffer &CB = *static_cast<HLCBuffer *>(&(HLM.GetCBuffer(i)));
    if (CB.GetConstants().size() == 0) {

      // FIXME: Can we avoid creating a fake variable here, since this empty
      // cbuffer presumably can't be used?

      // Create Fake variable for cbuffer which is empty.
      llvm::GlobalVariable *pGV = new llvm::GlobalVariable(
          *HLM.GetModule(), CBufferType, true,
          llvm::GlobalValue::ExternalLinkage, nullptr, CB.GetGlobalName());
      CB.SetGlobalSymbol(pGV);
    } else {
      bool bCreated = CreateCBufferVariable(CB, HLM, HandleTy);
      if (bCreated)
        ConstructCBufferAnnotation(CB, dxilTypeSys, AnnotationMap);
      else {

        // FIXME: Can we avoid creating a fake variable here, since this empty
        // cbuffer presumably can't be used?

        // Create Fake variable for cbuffer which is unused.
        llvm::GlobalVariable *pGV = new llvm::GlobalVariable(
            *HLM.GetModule(), CBufferType, true,
            llvm::GlobalValue::ExternalLinkage, nullptr, CB.GetGlobalName());
        CB.SetGlobalSymbol(pGV);
      }
    }
    // Clear the constants which useless now.
    CB.GetConstants().clear();
  }
}
} // namespace

namespace CGHLSLMSHelper {

// Align cbuffer offset in legacy mode (16 bytes per row).
unsigned AlignBufferOffsetInLegacy(unsigned offset, unsigned size,
                                   unsigned scalarSizeInBytes,
                                   bool bNeedNewRow) {
  if (unsigned remainder = (offset & 0xf)) {
    // Start from new row
    if (remainder + size > 16 || bNeedNewRow) {
      return offset + 16 - remainder;
    }
    // If not, naturally align data
    return RoundToAlign(offset, scalarSizeInBytes);
  }
  return offset;
}

// Translate RayQuery constructor.  From:
//  %call = call %"RayQuery<flags, constrayqueryflags<optional rayquery flags>>"
//  @<constructor>(%"RayQuery<flags>" %ptr)
// To:
//  i32 %handle = AllocateRayQuery2(i32 <IntrinsicOp::IOP_AllocateRayQuery>, i32
//  %flags, i32 %constrayqueryflags <0 if not given>) %gep = GEP
//  %"RayQuery<flags, constrayqueryflags>" %ptr, 0, 0 store i32* %gep, i32
//  %handle ; and replace uses of %call with %ptr
void TranslateRayQueryConstructor(HLModule &HLM) {
  llvm::Module &M = *HLM.GetModule();
  SmallVector<Function *, 4> Constructors;
  for (auto &F : M.functions()) {
    // Match templated RayQuery constructor instantiation by prefix and
    // signature. It should be impossible to achieve the same signature from
    // HLSL.
    if (!F.getName().startswith("\01??0?$RayQuery@$"))
      continue;
    llvm::Type *Ty = F.getReturnType();
    if (!Ty->isPointerTy() ||
        !dxilutil::IsHLSLRayQueryType(Ty->getPointerElementType()))
      continue;
    if (F.arg_size() != 1 || Ty != F.arg_begin()->getType())
      continue;
    Constructors.emplace_back(&F);
  }

  for (auto pConstructorFunc : Constructors) {
    llvm::IntegerType *i32Ty = llvm::Type::getInt32Ty(M.getContext());
    llvm::ConstantInt *i32Zero =
        llvm::ConstantInt::get(i32Ty, (uint64_t)0, false);

    // the third argument will default to 0 if the rayquery constructor doesn't
    // have a second template argument
    llvm::FunctionType *funcTy =
        llvm::FunctionType::get(i32Ty, {i32Ty, i32Ty, i32Ty}, false);
    unsigned opcode = (unsigned)IntrinsicOp::IOP_AllocateRayQuery;

    llvm::ConstantInt *opVal = llvm::ConstantInt::get(i32Ty, opcode, false);
    Function *opFunc =
        GetOrCreateHLFunction(M, funcTy, HLOpcodeGroup::HLIntrinsic, opcode);

    while (!pConstructorFunc->user_empty()) {
      Value *V = *pConstructorFunc->user_begin();
      llvm::CallInst *CI = cast<CallInst>(V); // Must be call
      llvm::Value *pThis = CI->getArgOperand(0);
      llvm::StructType *pRQType =
          cast<llvm::StructType>(pThis->getType()->getPointerElementType());
      DxilStructAnnotation *SA =
          HLM.GetTypeSystem().GetStructAnnotation(pRQType);
      DXASSERT(SA, "otherwise, could not find type annoation for RayQuery "
                   "specialization");
      DXASSERT((SA->GetNumTemplateArgs() == 1 &&
                SA->GetTemplateArgAnnotation(0).IsIntegral()) ||
                   (SA->GetNumTemplateArgs() == 2 &&
                    SA->GetTemplateArgAnnotation(0).IsIntegral() &&
                    SA->GetTemplateArgAnnotation(1).IsIntegral()),
               "otherwise, RayQuery has changed, or lacks template args");
      llvm::IRBuilder<> Builder(CI);
      llvm::Value *rayFlags =
          Builder.getInt32(SA->GetTemplateArgAnnotation(0).GetIntegral());
      // the default val of 0 will be assigned if there is no 2nd template arg
      llvm::Value *rayQueryFlags =
          Builder.getInt32(SA->GetTemplateArgAnnotation(1).GetIntegral());

      llvm::Value *Call = Builder.CreateCall(
          opFunc, {opVal, rayFlags, rayQueryFlags}, pThis->getName());

      llvm::Value *GEP = Builder.CreateInBoundsGEP(pThis, {i32Zero, i32Zero});
      Builder.CreateStore(Call, GEP);
      CI->replaceAllUsesWith(pThis);
      CI->eraseFromParent();
    }
    pConstructorFunc->eraseFromParent();
  }
}

void TranslateInputNodeRecordArgToHandle(
    hlsl::HLModule &HLM,
    llvm::MapVector<llvm::Argument *, NodeInputRecordProps> &NodeParams) {

  llvm::Module &Module = *HLM.GetModule();
  Type *HandleTy = HLM.GetOP()->GetNodeRecordHandleType();

  for (auto it : NodeParams) {
    NodeInputRecordProps Props = it.second;
    Argument *NodeParam = it.first;
    if (NodeParam->user_empty())
      continue;

    IRBuilder<> Builder(
        NodeParam->getParent()->getEntryBlock().getFirstInsertionPt());
    Value *Handle = CreateNodeInputRecordHandle(NodeParam, HLM, HandleTy,
                                                Builder, Props.MetadataIdx);
    Handle =
        CreateAnnotateNodeRecordHandle(HLM, Handle, Builder, Props.RecordInfo);
    llvm::Type *RetTy = NodeParam->getType()->getPointerElementType();
    Value *Res =
        HLM.EmitHLOperationCall(Builder, HLOpcodeGroup::HLCast,
                                (unsigned)HLCastOpcode::HandleToNodeRecordCast,
                                RetTy, {Handle}, Module);
    Builder.CreateStore(Res, NodeParam);
  }
}

void TranslateNodeOutputParamToHandle(
    hlsl::HLModule &HLM,
    llvm::MapVector<llvm::Argument *, NodeProps> &NodeParams) {
  Type *HandleTy = HLM.GetOP()->GetNodeHandleType();

  for (auto it : NodeParams) {
    NodeProps Props = it.second;
    Argument *NodeParam = it.first;
    if (NodeParam->user_empty())
      continue;

    IRBuilder<> Builder(
        NodeParam->getParent()->getEntryBlock().getFirstInsertionPt());
    Value *Handle =
        CreateNodeOutputHandle(HLM, HandleTy, Builder, Props.MetadataIdx);
    Handle = CreateAnnotateNodeHandle(HLM, Handle, Builder, Props.Info);
    Value *Res =
        HLM.EmitHLOperationCall(Builder, HLOpcodeGroup::HLCast,
                                (unsigned)HLCastOpcode::HandleToNodeOutputCast,
                                NodeParam->getType()->getPointerElementType(),
                                {Handle}, *HLM.GetModule());
    Builder.CreateStore(Res, NodeParam);
  }
}
} // namespace CGHLSLMSHelper

namespace {

bool BuildImmInit(Function *Ctor) {
  GlobalVariable *GV = nullptr;
  SmallVector<Constant *, 4> ImmList;
  bool allConst = true;
  for (inst_iterator I = inst_begin(Ctor), E = inst_end(Ctor); I != E; ++I) {
    if (StoreInst *SI = dyn_cast<StoreInst>(&(*I))) {
      Value *V = SI->getValueOperand();
      if (!isa<Constant>(V) || V->getType()->isPointerTy()) {
        allConst = false;
        break;
      }
      Value *Ptr = SI->getPointerOperand();
      if (GEPOperator *GepOp = dyn_cast<GEPOperator>(Ptr)) {
        Ptr = GepOp->getPointerOperand();
        if (GlobalVariable *pGV = dyn_cast<GlobalVariable>(Ptr)) {
          if (GV == nullptr)
            GV = pGV;
          else {
            DXASSERT(GV == pGV, "else pointer mismatch");
          }
        }
      }
      // If initializing an array, make sure init value type matches array
      // element type
      if (GV) {
        llvm::Type *GVElemTy = GV->getType()->getElementType();
        if (llvm::ArrayType *AT = dyn_cast<llvm::ArrayType>(GVElemTy)) {
          llvm::Type *ElTy = AT->getElementType();
          if (V->getType() != ElTy)
            return false;
        }
      }
      ImmList.emplace_back(cast<Constant>(V));
    } else {
      if (!isa<ReturnInst>(*I)) {
        allConst = false;
        break;
      }
    }
  }
  if (!allConst)
    return false;
  if (!GV)
    return false;

  llvm::Type *Ty = GV->getType()->getElementType();
  llvm::ArrayType *AT = dyn_cast<llvm::ArrayType>(Ty);
  // TODO: support other types.
  if (!AT)
    return false;
  if (ImmList.size() != AT->getNumElements())
    return false;
  Constant *Init = llvm::ConstantArray::get(AT, ImmList);
  GV->setInitializer(Init);
  return true;
}

void CallCtorFunctionsAtInsertPt(llvm::Module &M,
                                 llvm::SmallVector<llvm::Function *, 2> &Ctors,
                                 Instruction *InsertPt) {
  IRBuilder<> Builder(InsertPt);
  for (Function *Ctor : Ctors) {
    Builder.CreateCall(Ctor);
  }
}

void CollectFunctionCallers(Function *F, DenseSet<Function *> &Callers) {
  // worklist size max = call depth
  SmallVector<Function *, 8> worklist;
  worklist.push_back(F);
  // add callers
  while (worklist.size()) {
    Function *F = worklist.pop_back_val();
    for (User *U : F->users()) {
      if (CallInst *CI = dyn_cast<CallInst>(U)) {
        Function *Caller = CI->getParent()->getParent();
        if (Callers.insert(Caller).second == true) {
          // new caller
          worklist.push_back(Caller);
        }
      }
    }
  }
}

DenseSet<Function *> CollectExternalFunctionCallers(Module &M) {
  DenseSet<Function *> Callers;
  for (Function &F : M) {
    if (!F.isIntrinsic() && F.isDeclaration() &&
        hlsl::GetHLOpcodeGroup(&F) == hlsl::HLOpcodeGroup::NotHL) {
      CollectFunctionCallers(&F, Callers);
    }
  }
  return Callers;
}

// If static initializers contain calls to external functions, this can
// introduce inter-module init function ordering dependencies.  Some
// dependencies may even introduce contradictions.  Creating and implementing an
// intuitive standard approach to solve this is likely quite difficult.  Better
// to disallow the ambiguous and unlikely case for now.
bool IsValidCtorFunction(Function *F, DenseSet<Function *> &Callers) {
  return Callers.count(F) == 0;
}

void ReportInitStaticGlobalWithExternalFunction(
    clang::CodeGen ::CodeGenModule &CGM, StringRef name) {
  clang::DiagnosticsEngine &Diags = CGM.getDiags();
  unsigned DiagID = Diags.getCustomDiagID(
      clang::DiagnosticsEngine::Error, "Initializer for static global %0 makes "
                                       "disallowed call to external function.");
  std::string escaped;
  llvm::raw_string_ostream os(escaped);
  size_t end = name.find_first_of('@');
  if (end != StringRef::npos)
    name = name.substr(0, end);
  StringRef prefix = "\01??__E";
  if (name.startswith(prefix))
    name = name.substr(prefix.size());

  dxilutil::PrintEscapedString(name, os);
  Diags.Report(DiagID) << os.str();
}
} // namespace

namespace CGHLSLMSHelper {

void CollectCtorFunctions(llvm::Module &M, llvm::StringRef globalName,
                          llvm::SmallVector<llvm::Function *, 2> &Ctors,
                          clang::CodeGen::CodeGenModule &CGM) {
  // add global call to entry func
  GlobalVariable *GV = M.getGlobalVariable(globalName);
  if (!GV)
    return;
  ConstantArray *CA = dyn_cast<ConstantArray>(GV->getInitializer());
  if (!CA)
    return;

  DenseSet<Function *> Callers = CollectExternalFunctionCallers(M);

  bool allEvaluated = true;
  for (User::op_iterator i = CA->op_begin(), e = CA->op_end(); i != e; ++i) {
    if (isa<ConstantAggregateZero>(*i))
      continue;
    ConstantStruct *CS = cast<ConstantStruct>(*i);
    if (isa<ConstantPointerNull>(CS->getOperand(1)))
      continue;

    // Must have a function or null ptr.
    if (!isa<Function>(CS->getOperand(1)))
      continue;
    Function *Ctor = cast<Function>(CS->getOperand(1));
    DXASSERT(Ctor->getReturnType()->isVoidTy() && Ctor->arg_size() == 0,
             "function type must be void (void)");

    for (inst_iterator I = inst_begin(Ctor), E = inst_end(Ctor); I != E; ++I) {
      if (CallInst *CI = dyn_cast<CallInst>(&(*I))) {
        Function *F = CI->getCalledFunction();
        // Try to build imm initilizer.
        // If not work, add global call to entry func.
        if (BuildImmInit(F) == false) {
          allEvaluated = false;
          if (IsValidCtorFunction(F, Callers)) {
            Ctors.emplace_back(F);
          } else {
            ReportInitStaticGlobalWithExternalFunction(CGM, F->getName());
          }
        }
      } else {
        DXASSERT(isa<ReturnInst>(&(*I)),
                 "else invalid Global constructor function");
      }
    }
  }

  // If all globals constructors are replaced with initializers, just get rid
  // of the GV.
  if (allEvaluated) {
    GV->eraseFromParent();
  }
}

void ProcessCtorFunctions(llvm::Module &M,
                          llvm::SmallVector<llvm::Function *, 2> &Ctors,
                          llvm::Function *Entry,
                          llvm::Function *PatchConstantFn) {
  if (PatchConstantFn) {
    // static globals are independent for entry function and patch constant
    // function. Update static global in entry function will not affect
    // value in patch constant function. So just call ctors for patch
    // constant function too.
    CallCtorFunctionsAtInsertPt(
        M, Ctors, PatchConstantFn->getEntryBlock().getFirstInsertionPt());
    IRBuilder<> B(PatchConstantFn->getEntryBlock().getFirstInsertionPt());
    // For static globals which has const initialize value, copy it at
    // beginning of patch constant function to avoid use value updated by
    // entry function.
    for (GlobalVariable &GV : M.globals()) {
      if (GV.isConstant())
        continue;
      if (!GV.hasInitializer())
        continue;
      if (GV.getName() == "llvm.global_ctors")
        continue;
      Value *V = GV.getInitializer();
      if (isa<UndefValue>(V))
        continue;
      B.CreateStore(V, &GV);
    }
  }
  CallCtorFunctionsAtInsertPt(M, Ctors,
                              Entry->getEntryBlock().getFirstInsertionPt());
}

void FinishCBuffer(HLModule &HLM, llvm::Type *CBufferType,
                   std::unordered_map<Constant *, DxilFieldAnnotation>
                       &constVarAnnotationMap) {
  // Allocate constant buffers.
  AllocateDxilConstantBuffers(HLM, constVarAnnotationMap);
  // TODO: create temp variable for constant which has store use.

  // Create Global variable and type annotation for each CBuffer.
  ConstructCBuffer(HLM, CBufferType, constVarAnnotationMap);
}

void AddRegBindingsForResourceInConstantBuffer(
    HLModule &HLM,
    llvm::DenseMap<llvm::Constant *,
                   llvm::SmallVector<std::pair<DXIL::ResourceClass, unsigned>,
                                     1>> &constantRegBindingMap) {
  for (unsigned i = 0; i < HLM.GetCBuffers().size(); i++) {
    HLCBuffer &CB = *static_cast<HLCBuffer *>(&(HLM.GetCBuffer(i)));
    auto &Constants = CB.GetConstants();
    for (unsigned j = 0; j < Constants.size(); j++) {
      const std::unique_ptr<DxilResourceBase> &C = Constants[j];
      Constant *CGV = C->GetGlobalSymbol();
      auto &regBindings = constantRegBindingMap[CGV];
      if (regBindings.empty())
        continue;
      unsigned Srv = UINT_MAX;
      unsigned Uav = UINT_MAX;
      unsigned Sampler = UINT_MAX;
      for (auto it : regBindings) {
        unsigned RegNum = it.second;
        switch (it.first) {
        case DXIL::ResourceClass::SRV:
          Srv = RegNum;
          break;
        case DXIL::ResourceClass::UAV:
          Uav = RegNum;
          break;
        case DXIL::ResourceClass::Sampler:
          Sampler = RegNum;
          break;
        default:
          DXASSERT(0, "invalid resource class");
          break;
        }
      }
      HLM.AddRegBinding(CB.GetID(), j, Srv, Uav, Sampler);
    }
  }
}

// extension codegen.
void ExtensionCodeGen(HLModule &HLM, clang::CodeGen::CodeGenModule &CGM) {
  auto &Diags = CGM.getDiags();
  // Add root signature from a #define. Overrides root signature in function
  // attribute.
  {
    using Status = HLSLExtensionsCodegenHelper::CustomRootSignature::Status;
    HLSLExtensionsCodegenHelper::CustomRootSignature customRootSig;
    HLSLExtensionsCodegenHelper::CustomRootSignature::Status status =
        CGM.getCodeGenOpts().HLSLExtensionsCodegen->GetCustomRootSignature(
            &customRootSig);
    if (status == Status::FOUND) {
      DxilRootSignatureVersion rootSigVer;
      // set root signature version.
      if (CGM.getLangOpts().RootSigMinor == 0) {
        rootSigVer = hlsl::DxilRootSignatureVersion::Version_1_0;
      } else {
        DXASSERT(CGM.getLangOpts().RootSigMinor == 1,
                 "else CGMSHLSLRuntime Constructor needs to be updated");
        rootSigVer = hlsl::DxilRootSignatureVersion::Version_1_1;
      }

      RootSignatureHandle RootSigHandle;
      CompileRootSignature(
          customRootSig.RootSignature, Diags,
          clang::SourceLocation::getFromRawEncoding(
              customRootSig.EncodedSourceLocation),
          rootSigVer, DxilRootSignatureCompilationFlags::GlobalRootSignature,
          &RootSigHandle);
      if (!RootSigHandle.IsEmpty()) {
        RootSigHandle.EnsureSerializedAvailable();
        HLM.SetSerializedRootSignature(RootSigHandle.GetSerializedBytes(),
                                       RootSigHandle.GetSerializedSize());
      }
    }
  }
}
} // namespace CGHLSLMSHelper

namespace {
void ReportDisallowedTypeInExportParam(clang::CodeGen ::CodeGenModule &CGM,
                                       StringRef name) {
  clang::DiagnosticsEngine &Diags = CGM.getDiags();
  unsigned DiagID =
      Diags.getCustomDiagID(clang::DiagnosticsEngine::Error,
                            "Exported function %0 must not contain a "
                            "resource in parameter or return type.");
  std::string escaped;
  llvm::raw_string_ostream os(escaped);
  dxilutil::PrintEscapedString(name, os);
  Diags.Report(DiagID) << os.str();
}
} // namespace

namespace CGHLSLMSHelper {
void FinishClipPlane(HLModule &HLM, std::vector<Function *> &clipPlaneFuncList,
                     std::unordered_map<Value *, DebugLoc> &debugInfoMap,
                     clang::CodeGen::CodeGenModule &CGM) {
  bool bDebugInfo = CGM.getCodeGenOpts().getDebugInfo() ==
                    clang::CodeGenOptions::FullDebugInfo;
  Module &M = *HLM.GetModule();

  for (Function *F : clipPlaneFuncList) {
    DxilFunctionProps &props = HLM.GetDxilFunctionProps(F);
    IRBuilder<> Builder(F->getEntryBlock().getFirstInsertionPt());

    for (unsigned i = 0; i < DXIL::kNumClipPlanes; i++) {
      Value *clipPlane = props.ShaderProps.VS.clipPlanes[i];
      if (!clipPlane)
        continue;
      if (bDebugInfo) {
        Builder.SetCurrentDebugLocation(debugInfoMap[clipPlane]);
      }
      llvm::Type *Ty = clipPlane->getType()->getPointerElementType();
      // Constant *zeroInit = ConstantFP::get(Ty, 0);
      GlobalVariable *GV = new llvm::GlobalVariable(
          M, Ty, /*IsConstant*/ false, // constant false to store.
          llvm::GlobalValue::ExternalLinkage,
          /*InitVal*/ nullptr, Twine("SV_ClipPlane") + Twine(i));
      Value *initVal = Builder.CreateLoad(clipPlane);
      Builder.CreateStore(initVal, GV);
      props.ShaderProps.VS.clipPlanes[i] = GV;
    }
  }
}
} // namespace CGHLSLMSHelper

namespace {
void LowerExportFunctions(HLModule &HLM, clang::CodeGen::CodeGenModule &CGM,
                          dxilutil::ExportMap &exportMap,
                          StringMap<EntryFunctionInfo> &entryFunctionMap) {
  bool bIsLib = HLM.GetShaderModel()->IsLib();
  Module &M = *HLM.GetModule();

  if (bIsLib && !exportMap.empty()) {
    for (auto &it : entryFunctionMap) {
      if (HLM.HasDxilFunctionProps(it.second.Func)) {
        const DxilFunctionProps &props =
            HLM.GetDxilFunctionProps(it.second.Func);
        if (props.IsHS())
          exportMap.RegisterExportedFunction(
              props.ShaderProps.HS.patchConstantFunc);
      }
    }
  }

  if (bIsLib && !exportMap.empty()) {
    exportMap.BeginProcessing();
    for (Function &f : M.functions()) {
      if (f.isDeclaration() || f.isIntrinsic() ||
          GetHLOpcodeGroup(&f) != HLOpcodeGroup::NotHL)
        continue;
      exportMap.ProcessFunction(&f, true);
    }
    // TODO: add subobject export names here.
    if (!exportMap.EndProcessing()) {
      for (auto &name : exportMap.GetNameCollisions()) {
        clang::DiagnosticsEngine &Diags = CGM.getDiags();
        unsigned DiagID = Diags.getCustomDiagID(
            clang::DiagnosticsEngine::Error,
            "Export name collides with another export: %0");
        std::string escaped;
        llvm::raw_string_ostream os(escaped);
        dxilutil::PrintEscapedString(name, os);
        Diags.Report(DiagID) << os.str();
      }
      for (auto &name : exportMap.GetUnusedExports()) {
        clang::DiagnosticsEngine &Diags = CGM.getDiags();
        unsigned DiagID =
            Diags.getCustomDiagID(clang::DiagnosticsEngine::Error,
                                  "Could not find target for export: %0");
        std::string escaped;
        llvm::raw_string_ostream os(escaped);
        dxilutil::PrintEscapedString(name, os);
        Diags.Report(DiagID) << os.str();
      }
    }
  }

  for (auto &it : exportMap.GetFunctionRenames()) {
    Function *F = it.first;
    auto &renames = it.second;

    if (renames.empty())
      continue;

    // Rename the original, if necessary, then clone the rest
    if (renames.find(F->getName()) == renames.end())
      F->setName(*renames.begin());

    for (auto &itName : renames) {
      if (F->getName() != itName) {
        Function *pClone = CloneFunction(F, itName, &M, HLM.GetTypeSystem(),
                                         HLM.GetTypeSystem());
        // add DxilFunctionProps if entry
        if (HLM.HasDxilFunctionProps(F)) {
          DxilFunctionProps &props = HLM.GetDxilFunctionProps(F);
          auto newProps = llvm::make_unique<DxilFunctionProps>(props);
          HLM.AddDxilFunctionProps(pClone, newProps);
        }
      }
    }
  }
}

void CheckResourceParameters(HLModule &HLM,
                             clang::CodeGen::CodeGenModule &CGM) {
  Module &M = *HLM.GetModule();
  for (Function &f : M.functions()) {
    // Skip llvm intrinsics, non-external linkage, entry/patch constant func,
    // and HL intrinsics
    if (!f.isIntrinsic() &&
        f.getLinkage() == GlobalValue::LinkageTypes::ExternalLinkage &&
        !HLM.HasDxilFunctionProps(&f) && !HLM.IsPatchConstantShader(&f) &&
        GetHLOpcodeGroup(&f) == HLOpcodeGroup::NotHL) {
      // Verify no resources in param/return types
      if (dxilutil::ContainsHLSLObjectType(f.getReturnType())) {
        ReportDisallowedTypeInExportParam(CGM, f.getName());
        continue;
      }
      for (auto &Arg : f.args()) {
        if (dxilutil::ContainsHLSLObjectType(Arg.getType())) {
          ReportDisallowedTypeInExportParam(CGM, f.getName());
          break;
        }
      }
    }
  }
}

} // namespace

namespace CGHLSLMSHelper {

void UpdateLinkage(HLModule &HLM, clang::CodeGen::CodeGenModule &CGM,
                   dxilutil::ExportMap &exportMap,
                   StringMap<EntryFunctionInfo> &entryFunctionMap,
                   StringMap<PatchConstantInfo> &patchConstantFunctionMap) {

  bool bIsLib = HLM.GetShaderModel()->IsLib();
  Module &M = *HLM.GetModule();
  // Pin entry point and constant buffers, mark everything else internal.
  for (Function &f : M.functions()) {
    if (!bIsLib) {
      if (&f == HLM.GetEntryFunction() ||
          IsPatchConstantFunction(&f, patchConstantFunctionMap) ||
          f.isDeclaration()) {
        if (f.isDeclaration() && !f.isIntrinsic() &&
            GetHLOpcodeGroup(&f) == HLOpcodeGroup::NotHL) {
          clang::DiagnosticsEngine &Diags = CGM.getDiags();
          unsigned DiagID = Diags.getCustomDiagID(
              clang::DiagnosticsEngine::Error,
              "External function used in non-library profile: %0");
          std::string escaped;
          llvm::raw_string_ostream os(escaped);
          dxilutil::PrintEscapedString(f.getName(), os);
          Diags.Report(DiagID) << os.str();
          return;
        }
        f.setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
      } else {
        f.setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
      }
    }
    // Skip no inline functions, or all functions if we're not in the
    // AlwaysInline mode.
    if (f.hasFnAttribute(llvm::Attribute::NoInline) ||
        (CGM.getCodeGenOpts().getInlining() !=
             clang::CodeGenOptions::OnlyAlwaysInlining &&
         bIsLib))
      continue;
    // Always inline for used functions.
    if (!f.user_empty() && !f.isDeclaration())
      f.addFnAttr(llvm::Attribute::AlwaysInline);
  }

  LowerExportFunctions(HLM, CGM, exportMap, entryFunctionMap);

  if (CGM.getCodeGenOpts().ExportShadersOnly) {
    for (Function &f : M.functions()) {
      // Skip declarations, intrinsics, shaders, and non-external linkage
      if (f.isDeclaration() || f.isIntrinsic() ||
          GetHLOpcodeGroup(&f) != HLOpcodeGroup::NotHL ||
          HLM.HasDxilFunctionProps(&f) || HLM.IsPatchConstantShader(&f) ||
          f.getLinkage() != GlobalValue::LinkageTypes::ExternalLinkage)
        continue;
      // Mark non-shader user functions as InternalLinkage
      f.setLinkage(GlobalValue::LinkageTypes::InternalLinkage);
    }
  }
  // Now iterate hull shaders and make sure their corresponding patch constant
  // functions are marked ExternalLinkage:
  for (Function &f : M.functions()) {
    if (f.isDeclaration() || f.isIntrinsic() ||
        GetHLOpcodeGroup(&f) != HLOpcodeGroup::NotHL ||
        f.getLinkage() != GlobalValue::LinkageTypes::ExternalLinkage ||
        !HLM.HasDxilFunctionProps(&f))
      continue;
    DxilFunctionProps &props = HLM.GetDxilFunctionProps(&f);
    if (!props.IsHS())
      continue;
    Function *PCFunc = props.ShaderProps.HS.patchConstantFunc;
    if (PCFunc->getLinkage() != GlobalValue::LinkageTypes::ExternalLinkage)
      PCFunc->setLinkage(GlobalValue::LinkageTypes::ExternalLinkage);
  }

  // Disallow resource arguments in (non-entry) function exports
  // unless offline linking target.
  if (bIsLib &&
      HLM.GetShaderModel()->GetMinor() != ShaderModel::kOfflineMinor) {
    CheckResourceParameters(HLM, CGM);
  }
}
void FinishEntries(
    HLModule &HLM, const EntryFunctionInfo &Entry,
    clang::CodeGen::CodeGenModule &CGM,
    StringMap<EntryFunctionInfo> &entryFunctionMap,
    std::unordered_map<Function *, const clang::HLSLPatchConstantFuncAttr *>
        &HSEntryPatchConstantFuncAttr,
    StringMap<PatchConstantInfo> &patchConstantFunctionMap,
    std::unordered_map<Function *, std::unique_ptr<DxilFunctionProps>>
        &patchConstantFunctionPropsMap) {

  bool bIsLib = HLM.GetShaderModel()->IsLib();
  // Library don't have entry.
  if (!bIsLib) {
    SetEntryFunction(HLM, Entry.Func, CGM);

    // If at this point we haven't determined the entry function it's an error.
    if (HLM.GetEntryFunction() == nullptr) {
      assert(CGM.getDiags().hasErrorOccurred() &&
             "else SetEntryFunction should have reported this condition");
      return;
    }

    // In back-compat mode (with /Gec flag) create a static global for each
    // const global to allow writing to it.
    // TODO: Verfiy the behavior of static globals in hull shader
    if (CGM.getLangOpts().EnableDX9CompatMode &&
        CGM.getLangOpts().HLSLVersion <= hlsl::LangStd::v2016)
      CreateWriteEnabledStaticGlobals(HLM.GetModule(), HLM.GetEntryFunction());
    if (HLM.GetShaderModel()->IsHS()) {
      SetPatchConstantFunction(Entry, HSEntryPatchConstantFuncAttr,
                               patchConstantFunctionMap,
                               patchConstantFunctionPropsMap, HLM, CGM);
    }
  } else {
    for (auto &it : entryFunctionMap) {
      // skip clone if RT entry
      if (HLM.GetDxilFunctionProps(it.second.Func).IsRay())
        continue;

      // TODO: change flattened function names to dx.entry.<name>:
      // std::string entryName = (Twine(dxilutil::EntryPrefix) +
      // it.getKey()).str();
      CloneShaderEntry(it.second.Func, it.getKey(), HLM);

      auto AttrIter = HSEntryPatchConstantFuncAttr.find(it.second.Func);
      if (AttrIter != HSEntryPatchConstantFuncAttr.end()) {
        SetPatchConstantFunctionWithAttr(
            it.second, AttrIter->second, patchConstantFunctionMap,
            patchConstantFunctionPropsMap, HLM, CGM);
      }
    }
  }
}
} // namespace CGHLSLMSHelper

namespace CGHLSLMSHelper {
void FinishIntrinsics(
    HLModule &HLM, std::vector<std::pair<Function *, unsigned>> &intrinsicMap,
    DxilObjectProperties &objectProperties) {
  // Lower getResourceHeap before AddOpcodeParamForIntrinsics to skip automatic
  // lower for getResourceFromHeap.
  LowerGetResourceFromHeap(HLM, intrinsicMap);
  // Lower bitcast use of CBV into cbSubscript.
  LowerDynamicCBVUseToHandle(HLM, objectProperties);

  // translate opcode into parameter for intrinsic functions
  // Do this before CloneShaderEntry and TranslateRayQueryConstructor to avoid
  // update valToResPropertiesMap for cloned inst.
  AddOpcodeParamForIntrinsics(HLM, intrinsicMap, objectProperties);
}

// Add the dx.break temporary intrinsic and create Call Instructions
// to it for each branch that requires the artificial conditional.
void AddDxBreak(Module &M,
                const SmallVector<llvm::BranchInst *, 16> &DxBreaks) {
  if (DxBreaks.empty())
    return;

  // Collect functions that make use of any wave operations
  // Only they will need the dx.break condition added
  SmallPtrSet<Function *, 16> WaveUsers;
  for (Function &F : M.functions()) {
    HLOpcodeGroup opgroup = hlsl::GetHLOpcodeGroup(&F);
    if (F.isDeclaration() && IsHLWaveSensitive(&F) &&
        (opgroup == HLOpcodeGroup::HLIntrinsic ||
         opgroup == HLOpcodeGroup::HLExtIntrinsic)) {
      for (User *U : F.users()) {
        CallInst *CI = cast<CallInst>(U);
        WaveUsers.insert(CI->getParent()->getParent());
      }
    }
  }

  // If there are no wave users, not even the function declaration is needed
  if (WaveUsers.empty())
    return;

  // Create the dx.break function
  FunctionType *FT =
      llvm::FunctionType::get(llvm::Type::getInt1Ty(M.getContext()), false);
  Function *func =
      cast<llvm::Function>(M.getOrInsertFunction(DXIL::kDxBreakFuncName, FT));
  func->addFnAttr(Attribute::AttrKind::NoUnwind);

  // For all break branches recorded previously, if the function they are in
  // makes any use of a wave op, it may need to be artificially conditional.
  // Make it so now. The CleanupDxBreak pass will remove those that aren't
  // needed when more is known.
  for (llvm::BranchInst *BI : DxBreaks) {
    if (WaveUsers.count(BI->getParent()->getParent())) {
      CallInst *Call = CallInst::Create(FT, func, ArrayRef<Value *>(), "", BI);
      BI->setCondition(Call);
      if (!BI->getMetadata(DXIL::kDxBreakMDName)) {
        BI->setMetadata(DXIL::kDxBreakMDName,
                        llvm::MDNode::get(BI->getContext(), {}));
      }
    }
  }
}

} // namespace CGHLSLMSHelper

namespace CGHLSLMSHelper {

ScopeInfo::ScopeInfo(Function *F, clang::SourceLocation loc)
    : maxRetLevel(0), bAllReturnsInIf(true), sourceLoc(loc) {
  Scope FuncScope;
  FuncScope.kind = Scope::ScopeKind::FunctionScope;
  FuncScope.EndScopeBB = nullptr;
  FuncScope.bWholeScopeReturned = false;
  // Make it 0 to avoid check when get parent.
  // All loop on scopes should check kind != FunctionScope.
  FuncScope.parentScopeIndex = 0;
  scopes.emplace_back(FuncScope);
  scopeStack.emplace_back(0);
}

// When all returns is inside if which is not nested, the flow is still
// structurized even there're more than one return.
bool ScopeInfo::CanSkipStructurize() {
  return (bAllReturnsInIf && maxRetLevel < 2) || rets.size() < 2;
}

void ScopeInfo::AddScope(Scope::ScopeKind k, BasicBlock *endScopeBB) {
  Scope Scope;
  Scope.kind = k;
  Scope.bWholeScopeReturned = false;
  Scope.EndScopeBB = endScopeBB;
  Scope.parentScopeIndex = scopeStack.back();
  scopeStack.emplace_back(scopes.size());
  scopes.emplace_back(Scope);
}

void ScopeInfo::AddIf(BasicBlock *endIfBB) {
  AddScope(Scope::ScopeKind::IfScope, endIfBB);
}

void ScopeInfo::AddSwitch(BasicBlock *endSwitch) {
  AddScope(Scope::ScopeKind::SwitchScope, endSwitch);
}

void ScopeInfo::AddLoop(BasicBlock *loopContinue, BasicBlock *endLoop) {
  AddScope(Scope::ScopeKind::LoopScope, endLoop);
  scopes.back().loopContinueBB = loopContinue;
}

void ScopeInfo::AddRet(BasicBlock *bbWithRet) {
  Scope RetScope;
  RetScope.kind = Scope::ScopeKind::ReturnScope;
  RetScope.EndScopeBB = bbWithRet;
  RetScope.parentScopeIndex = scopeStack.back();
  // - 1 for function scope which is at scopeStack[0].
  unsigned retLevel = scopeStack.size() - 1;
  // save max nested level for ret.
  maxRetLevel = std::max<unsigned>(maxRetLevel, retLevel);
  bool bGotLoopOrSwitch = false;
  for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); it++) {
    unsigned idx = *it;
    Scope &S = scopes[idx];
    switch (S.kind) {
    default:
      break;
    case Scope::ScopeKind::LoopScope:
    case Scope::ScopeKind::SwitchScope:
      bGotLoopOrSwitch = true;
      // For return inside loop and switch, can just break.
      RetScope.parentScopeIndex = idx;
      break;
    }
    if (bGotLoopOrSwitch)
      break;
  }
  bAllReturnsInIf &= !bGotLoopOrSwitch;
  // return finish current scope.
  RetScope.bWholeScopeReturned = true;
  // save retScope to rets.
  rets.emplace_back(scopes.size());
  scopes.emplace_back(RetScope);
  // Don't need to put retScope to stack since it cannot nested other scopes.
}

Scope &ScopeInfo::EndScope(bool bScopeFinishedWithRet) {
  unsigned idx = scopeStack.pop_back_val();
  Scope &Scope = GetScope(idx);
  // If whole stmt is finished and end scope bb has not used(nothing branch to
  // it). Then the whole scope is returned.
  Scope.bWholeScopeReturned =
      bScopeFinishedWithRet && Scope.EndScopeBB->user_empty();
  return Scope;
}

Scope &ScopeInfo::GetScope(unsigned i) { return scopes[i]; }

void ScopeInfo::LegalizeWholeReturnedScope() {
  // legalize scopes which whole scope returned.
  // When whole scope is returned, the endScopeBB will be deleted in codeGen.
  // Here update it to parent scope's endScope.
  // Since the scopes are in order, so it will automatic update to the final
  // target. A->B->C will just get A->C.
  for (auto &S : scopes) {
    if (S.bWholeScopeReturned && S.kind != Scope::ScopeKind::ReturnScope) {
      S.EndScopeBB = scopes[S.parentScopeIndex].EndScopeBB;
    }
  }
}

void Scope::dump() {
  auto &OS = llvm::dbgs();
  switch (kind) {
  case ScopeKind::IfScope:
    OS << "If\n";
    break;
  case ScopeKind::LoopScope:
    OS << "Loop\n";
    break;
  case ScopeKind::ReturnScope:
    OS << "Return\n";
    break;
  case ScopeKind::SwitchScope:
    OS << "Switch\n";
    break;
  case ScopeKind::FunctionScope:
    OS << "Function\n";
    break;
  }
  if (kind == ScopeKind::FunctionScope)
    return;

  OS << "parent:" << parentScopeIndex << "\n";
  if (bWholeScopeReturned)
    OS << "whole scope returned\n";
  OS << "endBB:";
  EndScopeBB->printAsOperand(OS);
  OS << "\n";
}

void ScopeInfo::dump() {
  auto &OS = llvm::dbgs();
  for (unsigned i = 0; i < scopes.size(); ++i) {
    Scope &scope = scopes[i];
    OS << "Scope:" << i << "\n";
    scope.dump();
  }
}

} // namespace CGHLSLMSHelper

namespace {

void updateEndScope(
    ScopeInfo &ScopeInfo,
    DenseMap<BasicBlock *, SmallVector<unsigned, 2>> &EndBBToScopeIndexMap,
    BasicBlock *oldEndScope, BasicBlock *newEndScope) {
  auto it = EndBBToScopeIndexMap.find(oldEndScope);
  DXASSERT(it != EndBBToScopeIndexMap.end(),
           "fail to find endScopeBB in EndBBToScopeIndexMap");
  SmallVector<unsigned, 2> &scopeList = it->second;
  // Update even when not share endBB with other scope.
  // The endBB might be used by nested returns like the return 1 in
  // if (b == 77)
  //  return 0;
  // else if (b != 3)
  //  return 1;
  for (unsigned i : scopeList) {
    Scope &S = ScopeInfo.GetScope(i);
    // Don't update return endBB, because that is the Block has return branch.
    if (S.kind != Scope::ScopeKind::ReturnScope)
      S.EndScopeBB = newEndScope;
  }
  EndBBToScopeIndexMap[newEndScope] = scopeList;
}

// Init ret value with undef to make sure it will not live thru loop inside
// callers.
// Because structurize return, the flow is controled by bIsReturned. The
// semantic is the same as multiple return, but without konwledge of
// bIsReturend, some path for structrized flow will have ret value not
// initialized.
// When function is called inside loop, ret value will live across the loop
// after inline.
void InitRetValue(BasicBlock *exitBB) {
  Value *RetValPtr = nullptr;
  if (ReturnInst *RI = dyn_cast<ReturnInst>(exitBB->getTerminator())) {
    if (Value *RetV = RI->getReturnValue()) {
      if (LoadInst *LI = dyn_cast<LoadInst>(RetV)) {
        RetValPtr = LI->getPointerOperand();
      }
    }
  }
  if (!RetValPtr)
    return;
  if (AllocaInst *RetVAlloc = dyn_cast<AllocaInst>(RetValPtr)) {
    IRBuilder<> B(RetVAlloc->getNextNode());
    Type *Ty = RetVAlloc->getAllocatedType();
    Value *Init = UndefValue::get(Ty);
    if (Ty->isAggregateType()) {
      // TODO: support aggreagate type and out parameters.
      // Skip it here will cause undef on phi which the incoming path should
      // never hit.
    } else {
      B.CreateStore(Init, RetVAlloc);
    }
  }
}

static void ChangePredBranch(BasicBlock *BB, BasicBlock *NewBB) {
  for (auto predIt = pred_begin(BB); predIt != pred_end(BB);) {
    BasicBlock *Pred = *(predIt++);
    TerminatorInst *TI = Pred->getTerminator();
    TI->replaceUsesOfWith(BB, NewBB);
  }
}

// For functions has multiple returns like
// float foo(float a, float b, float c) {
//   float r = c;
//   if (a > 0) {
//      if (b > 0) {
//        return -1;
//      }
//      ***
//   }
//   ...
//   return r;
// }
// transform into
// float foo(float a, float b, float c) {
//   bool bRet = false;
//   float retV;
//   float r = c;
//   if (a > 0) {
//      if (b > 0) {
//        bRet = true;
//        retV = -1;
//      }
//      if (!bRet) {
//        ***
//      }
//   }
//   if (!bRet) {
//     ...
//     retV = r;
//   }
//   return vRet;
// }
void StructurizeMultiRetFunction(Function *F, clang::DiagnosticsEngine &Diags,
                                 ScopeInfo &ScopeInfo, bool bWaveEnabledStage,
                                 SmallVector<BranchInst *, 16> &DxBreaks) {

  if (ScopeInfo.CanSkipStructurize())
    return;

  // If there are cleanup blocks generated for lifetime markers, do
  // not structurize returns. The scope info recorded is no longer correct.
  if (ScopeInfo.HasCleanupBlocks()) {
    Diags.Report(
        ScopeInfo.GetSourceLocation(),
        clang::diag::warn_hlsl_structurize_exits_lifetime_markers_conflict)
        << F->getName();
    return;
  }

  // Get bbWithRets.
  auto &rets = ScopeInfo.GetRetScopes();

  IRBuilder<> B(F->getEntryBlock().begin());

  Scope &FunctionScope = ScopeInfo.GetScope(0);

  Type *boolTy = Type::getInt1Ty(F->getContext());
  Constant *cTrue = ConstantInt::get(boolTy, 1);
  Constant *cFalse = ConstantInt::get(boolTy, 0);
  // bool bIsReturned = false;
  AllocaInst *bIsReturned = B.CreateAlloca(boolTy, nullptr, "bReturned");
  B.CreateStore(cFalse, bIsReturned);

  Scope &RetScope = ScopeInfo.GetScope(rets[0]);
  BasicBlock *exitBB = RetScope.EndScopeBB->getTerminator()->getSuccessor(0);
  FunctionScope.EndScopeBB = exitBB;
  // Find alloca for retunr val and init it to avoid undef after guard code with
  // bIsReturned.
  InitRetValue(exitBB);

  ScopeInfo.LegalizeWholeReturnedScope();

  // Map from endScopeBB to scope index.
  // When 2 scopes share same endScopeBB, need to update endScopeBB after
  // structurize.
  DenseMap<BasicBlock *, SmallVector<unsigned, 2>> EndBBToScopeIndexMap;
  auto &scopes = ScopeInfo.GetScopes();
  for (unsigned i = 0; i < scopes.size(); i++) {
    Scope &S = scopes[i];
    EndBBToScopeIndexMap[S.EndScopeBB].emplace_back(i);
  }

  DenseSet<unsigned> guardedSet;

  for (auto it = rets.begin(); it != rets.end(); it++) {
    unsigned scopeIndex = *it;
    Scope *pCurScope = &ScopeInfo.GetScope(scopeIndex);
    Scope *pRetParentScope = &ScopeInfo.GetScope(pCurScope->parentScopeIndex);
    // skip ret not in nested control flow.
    if (pRetParentScope->kind == Scope::ScopeKind::FunctionScope)
      continue;

    do {
      BasicBlock *BB = pCurScope->EndScopeBB;
      // exit when scope is processed.
      if (guardedSet.count(scopeIndex))
        break;
      guardedSet.insert(scopeIndex);

      Scope *pParentScope = &ScopeInfo.GetScope(pCurScope->parentScopeIndex);
      BasicBlock *EndBB = pParentScope->EndScopeBB;
      // When whole scope returned, just branch to endScope of parent.
      if (pCurScope->bWholeScopeReturned) {
        // For ret, just branch to endScope of parent.
        if (pCurScope->kind == Scope::ScopeKind::ReturnScope) {
          BasicBlock *retBB = pCurScope->EndScopeBB;
          TerminatorInst *retBr = retBB->getTerminator();
          IRBuilder<> B(retBr);
          // Set bReturned to true.
          B.CreateStore(cTrue, bIsReturned);
          if (bWaveEnabledStage &&
              pParentScope->kind == Scope::ScopeKind::LoopScope) {
            BranchInst *BI =
                B.CreateCondBr(cTrue, EndBB, pParentScope->loopContinueBB);
            DxBreaks.emplace_back(BI);
            retBr->eraseFromParent();
          } else {
            // Update branch target.
            retBr->setSuccessor(0, EndBB);
          }
        }
        // For other scope, do nothing. Since whole scope is returned.
        // Just flow naturally to parent scope.
      } else {
        // When only part scope returned.
        // Use bIsReturned to guard to part which not returned.
        switch (pParentScope->kind) {
        case Scope::ScopeKind::ReturnScope:
          DXASSERT(0, "return scope must get whole scope returned.");
          break;
        case Scope::ScopeKind::FunctionScope:
        case Scope::ScopeKind::IfScope: {
          // inside if.
          // if (!bReturned) {
          //   rest of if or else.
          // }
          BasicBlock *CmpBB = BasicBlock::Create(BB->getContext(),
                                                 "bReturned.cmp.false", F, BB);

          // Make BB preds go to cmpBB. Do this instead of replaceAllUsesWith
          // because BB could have PHI nodes that reference it.
          ChangePredBranch(BB, CmpBB);

          // Update endscopeBB to CmpBB for scopes which has BB as endscope.
          updateEndScope(ScopeInfo, EndBBToScopeIndexMap, BB, CmpBB);

          IRBuilder<> B(CmpBB);
          Value *isRetured = B.CreateLoad(bIsReturned, "bReturned.load");
          Value *notReturned =
              B.CreateICmpNE(isRetured, cFalse, "bReturned.not");
          B.CreateCondBr(notReturned, EndBB, BB);
        } break;
        default: {
          // inside switch/loop
          // if (bReturned) {
          //   br endOfScope.
          // }
          BasicBlock *CmpBB =
              BasicBlock::Create(BB->getContext(), "bReturned.cmp.true", F, BB);
          BasicBlock *BreakBB =
              BasicBlock::Create(BB->getContext(), "bReturned.break", F, BB);
          ChangePredBranch(BB, CmpBB);

          // Update endscopeBB to CmpBB for scopes which has BB as endscope.
          updateEndScope(ScopeInfo, EndBBToScopeIndexMap, BB, CmpBB);

          IRBuilder<> B(CmpBB);
          Value *isReturned = B.CreateLoad(bIsReturned, "bReturned.load");
          isReturned = B.CreateICmpEQ(isReturned, cTrue, "bReturned.true");
          B.CreateCondBr(isReturned, BreakBB, BB);

          B.SetInsertPoint(BreakBB);
          if (bWaveEnabledStage &&
              pParentScope->kind == Scope::ScopeKind::LoopScope) {
            BranchInst *BI =
                B.CreateCondBr(cTrue, EndBB, pParentScope->loopContinueBB);
            DxBreaks.emplace_back(BI);
          } else {
            B.CreateBr(EndBB);
          }
        } break;
        }
      }

      scopeIndex = pCurScope->parentScopeIndex;
      pCurScope = &ScopeInfo.GetScope(scopeIndex);
      // done when reach function scope.
    } while (pCurScope->kind != Scope::ScopeKind::FunctionScope);
  }
}
} // namespace

namespace CGHLSLMSHelper {
void StructurizeMultiRet(Module &M, clang::CodeGen::CodeGenModule &CGM,
                         DenseMap<Function *, ScopeInfo> &ScopeMap,
                         bool bWaveEnabledStage,
                         SmallVector<BranchInst *, 16> &DxBreaks) {
  if (CGM.getCodeGenOpts().HLSLExtensionsCodegen) {
    if (!CGM.getCodeGenOpts().HLSLExtensionsCodegen->IsOptionEnabled(
            hlsl::options::TOGGLE_STRUCTURIZE_RETURNS))
      return;
  } else {
    if (!CGM.getCodeGenOpts().HLSLOptimizationToggles.IsEnabled(
            hlsl::options::TOGGLE_STRUCTURIZE_RETURNS))
      return;
  }

  for (Function &F : M) {
    if (F.isDeclaration())
      continue;
    auto it = ScopeMap.find(&F);
    if (it == ScopeMap.end())
      continue;

    StructurizeMultiRetFunction(&F, CGM.getDiags(), it->second,
                                bWaveEnabledStage, DxBreaks);
  }
}

bool DxilObjectProperties::AddResource(llvm::Value *V,
                                       const hlsl::DxilResourceProperties &RP) {
  if (RP.isValid()) {
    DXASSERT(!GetResource(V).isValid() || GetResource(V) == RP,
             "otherwise, property conflict");
    resMap[V] = RP;
    return true;
  }
  return false;
}
bool DxilObjectProperties::IsResource(llvm::Value *V) {
  return resMap.count(V) != 0;
}
hlsl::DxilResourceProperties DxilObjectProperties::GetResource(llvm::Value *V) {
  auto it = resMap.find(V);
  if (it != resMap.end())
    return it->second;
  return DxilResourceProperties();
}
void DxilObjectProperties::updateCoherence(llvm::Value *V,
                                           bool updateGloballyCoherent,
                                           bool updateReorderCoherent) {
  auto it = resMap.find(V);
  if (it == resMap.end())
    return;

  if (updateGloballyCoherent)
    it->second.Basic.IsGloballyCoherent ^= 1;
  if (updateReorderCoherent)
    it->second.Basic.IsReorderCoherent ^= 1;
}

} // namespace CGHLSLMSHelper
