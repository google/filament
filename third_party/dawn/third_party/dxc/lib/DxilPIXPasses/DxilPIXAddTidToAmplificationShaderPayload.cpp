///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilPIXAddTidToAmplificationShaderPayload.cpp                             //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilUtil.h"

#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DxilPIXPasses/DxilPIXPasses.h"

#include "llvm/IR/InstIterator.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Transforms/Utils/Local.h"

#include "PixPassHelpers.h"

using namespace llvm;
using namespace hlsl;
using namespace PIXPassHelpers;

class DxilPIXAddTidToAmplificationShaderPayload : public ModulePass {
  uint32_t m_DispatchArgumentY = 1;
  uint32_t m_DispatchArgumentZ = 1;

public:
  static char ID; // Pass identification, replacement for typeid
  DxilPIXAddTidToAmplificationShaderPayload() : ModulePass(ID) {}
  StringRef getPassName() const override {
    return "DXIL Add flat thread id to payload from AS to MS";
  }
  bool runOnModule(Module &M) override;
  void applyOptions(PassOptions O) override;
};

void DxilPIXAddTidToAmplificationShaderPayload::applyOptions(PassOptions O) {
  GetPassOptionUInt32(O, "dispatchArgY", &m_DispatchArgumentY, 1);
  GetPassOptionUInt32(O, "dispatchArgZ", &m_DispatchArgumentZ, 1);
}

void AddValueToExpandedPayload(OP *HlslOP, llvm::IRBuilder<> &B,
                               AllocaInst *NewStructAlloca,
                               unsigned int expandedValueIndex, Value *value) {
  Constant *Zero32Arg = HlslOP->GetU32Const(0);
  SmallVector<Value *, 2> IndexToAppendedValue;
  IndexToAppendedValue.push_back(Zero32Arg);
  IndexToAppendedValue.push_back(HlslOP->GetU32Const(expandedValueIndex));
  auto *PointerToEmbeddedNewValue = B.CreateInBoundsGEP(
      NewStructAlloca, IndexToAppendedValue,
      "PointerToEmbeddedNewValue" + std::to_string(expandedValueIndex));
  B.CreateStore(value, PointerToEmbeddedNewValue);
}

void CopyAggregate(IRBuilder<> &B, Type *Ty, Value *Source, Value *Dest,
                   ArrayRef<Value *> GEPIndices) {
  if (StructType *ST = dyn_cast<StructType>(Ty)) {
    SmallVector<Value *, 16> StructIndices;
    StructIndices.append(GEPIndices.begin(), GEPIndices.end());
    StructIndices.push_back(nullptr);
    for (unsigned j = 0; j < ST->getNumElements(); ++j) {
      StructIndices.back() = B.getInt32(j);
      CopyAggregate(B, ST->getElementType(j), Source, Dest, StructIndices);
    }
  } else if (ArrayType *AT = dyn_cast<ArrayType>(Ty)) {
    SmallVector<Value *, 16> StructIndices;
    StructIndices.append(GEPIndices.begin(), GEPIndices.end());
    StructIndices.push_back(nullptr);
    for (unsigned j = 0; j < AT->getNumElements(); ++j) {
      StructIndices.back() = B.getInt32(j);
      CopyAggregate(B, AT->getArrayElementType(), Source, Dest, StructIndices);
    }
  } else {
    auto *SourceGEP = B.CreateGEP(Source, GEPIndices, "CopyStructSourceGEP");
    Value *Val = B.CreateLoad(SourceGEP, "CopyStructLoad");
    auto *DestGEP = B.CreateGEP(Dest, GEPIndices, "CopyStructDestGEP");
    B.CreateStore(Val, DestGEP, "CopyStructStore");
  }
}

bool DxilPIXAddTidToAmplificationShaderPayload::runOnModule(Module &M) {
  DxilModule &DM = M.GetOrCreateDxilModule();
  LLVMContext &Ctx = M.getContext();
  OP *HlslOP = DM.GetOP();
  llvm::Function *entryFunction = PIXPassHelpers::GetEntryFunction(DM);
  for (inst_iterator I = inst_begin(entryFunction), E = inst_end(entryFunction);
       I != E; ++I) {
    if (hlsl::OP::IsDxilOpFuncCallInst(&*I, hlsl::OP::OpCode::DispatchMesh)) {
      DxilInst_DispatchMesh DispatchMesh(&*I);
      Type *OriginalPayloadStructPointerType =
          DispatchMesh.get_payload()->getType();
      Type *OriginalPayloadStructType =
          OriginalPayloadStructPointerType->getPointerElementType();
      ExpandedStruct expanded =
          ExpandStructType(Ctx, OriginalPayloadStructType);

      llvm::IRBuilder<> B(&*I);

      auto *NewStructAlloca =
          B.CreateAlloca(expanded.ExpandedPayloadStructType,
                         HlslOP->GetU32Const(1), "NewPayload");
      NewStructAlloca->setAlignment(4);
      auto PayloadType =
          llvm::dyn_cast<PointerType>(DispatchMesh.get_payload()->getType());
      SmallVector<Value *, 16> GEPIndices;
      GEPIndices.push_back(B.getInt32(0));
      CopyAggregate(B, PayloadType->getPointerElementType(),
                    DispatchMesh.get_payload(), NewStructAlloca, GEPIndices);

      Constant *Zero32Arg = HlslOP->GetU32Const(0);
      Constant *One32Arg = HlslOP->GetU32Const(1);
      Constant *Two32Arg = HlslOP->GetU32Const(2);

      auto GroupIdFunc =
          HlslOP->GetOpFunc(DXIL::OpCode::GroupId, Type::getInt32Ty(Ctx));
      Constant *GroupIdOpcode =
          HlslOP->GetU32Const((unsigned)DXIL::OpCode::GroupId);
      auto *GroupIdX =
          B.CreateCall(GroupIdFunc, {GroupIdOpcode, Zero32Arg}, "GroupIdX");
      auto *GroupIdY =
          B.CreateCall(GroupIdFunc, {GroupIdOpcode, One32Arg}, "GroupIdY");
      auto *GroupIdZ =
          B.CreateCall(GroupIdFunc, {GroupIdOpcode, Two32Arg}, "GroupIdZ");

      // FlatGroupID = z + y*numZ + x*numY*numZ
      // Where x,y,z are the group ID components, and numZ and numY are the
      // corresponding AS group-count arguments to the DispatchMesh Direct3D API
      auto *GroupYxNumZ = B.CreateMul(
          GroupIdY, HlslOP->GetU32Const(m_DispatchArgumentZ), "GroupYxNumZ");
      auto *FlatGroupNumZY =
          B.CreateAdd(GroupIdZ, GroupYxNumZ, "FlatGroupNumZY");
      auto *GroupXxNumYZ = B.CreateMul(
          GroupIdX,
          HlslOP->GetU32Const(m_DispatchArgumentY * m_DispatchArgumentZ),
          "GroupXxNumYZ");
      auto *FlatGroupID =
          B.CreateAdd(GroupXxNumYZ, FlatGroupNumZY, "FlatGroupID");

      // The ultimate goal is a single unique thread ID for this AS thread.
      // So take the flat group number, multiply it by the number of
      // threads per group...
      auto *FlatGroupIDWithSpaceForThreadInGroupId = B.CreateMul(
          FlatGroupID,
          HlslOP->GetU32Const(DM.GetNumThreads(0) * DM.GetNumThreads(1) *
                              DM.GetNumThreads(2)),
          "FlatGroupIDWithSpaceForThreadInGroupId");

      auto *FlattenedThreadIdInGroupFunc = HlslOP->GetOpFunc(
          DXIL::OpCode::FlattenedThreadIdInGroup, Type::getInt32Ty(Ctx));
      Constant *FlattenedThreadIdInGroupOpcode =
          HlslOP->GetU32Const((unsigned)DXIL::OpCode::FlattenedThreadIdInGroup);
      auto FlatThreadIdInGroup = B.CreateCall(FlattenedThreadIdInGroupFunc,
                                              {FlattenedThreadIdInGroupOpcode},
                                              "FlattenedThreadIdInGroup");

      // ...and add the flat thread id:
      auto *FlatId = B.CreateAdd(FlatGroupIDWithSpaceForThreadInGroupId,
                                 FlatThreadIdInGroup, "FlatId");

      AddValueToExpandedPayload(
          HlslOP, B, NewStructAlloca,
          expanded.ExpandedPayloadStructType->getStructNumElements() - 3,
          FlatId);
      AddValueToExpandedPayload(
          HlslOP, B, NewStructAlloca,
          expanded.ExpandedPayloadStructType->getStructNumElements() - 2,
          DispatchMesh.get_threadGroupCountY());
      AddValueToExpandedPayload(
          HlslOP, B, NewStructAlloca,
          expanded.ExpandedPayloadStructType->getStructNumElements() - 1,
          DispatchMesh.get_threadGroupCountZ());

      auto DispatchMeshFn = HlslOP->GetOpFunc(
          DXIL::OpCode::DispatchMesh, expanded.ExpandedPayloadStructPtrType);
      Constant *DispatchMeshOpcode =
          HlslOP->GetU32Const((unsigned)DXIL::OpCode::DispatchMesh);
      B.CreateCall(DispatchMeshFn,
                   {DispatchMeshOpcode, DispatchMesh.get_threadGroupCountX(),
                    DispatchMesh.get_threadGroupCountY(),
                    DispatchMesh.get_threadGroupCountZ(), NewStructAlloca});
      I->removeFromParent();
      delete &*I;
      // Validation requires exactly one DispatchMesh in an AS, so we can exit
      // after the first one:
      DM.ReEmitDxilResources();
      return true;
    }
  }

  return false;
}

char DxilPIXAddTidToAmplificationShaderPayload::ID = 0;

ModulePass *llvm::createDxilPIXAddTidToAmplificationShaderPayloadPass() {
  return new DxilPIXAddTidToAmplificationShaderPayload();
}

INITIALIZE_PASS(DxilPIXAddTidToAmplificationShaderPayload,
                "hlsl-dxil-PIX-add-tid-to-as-payload",
                "HLSL DXIL Add flat thread id to payload from AS to MS", false,
                false)
