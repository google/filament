///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilAnnotateWithVirtualRegister.cpp                                       //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Annotates the llvm instructions with a virtual register number to be used //
// during PIX debugging.                                                     //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <memory>

#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilUtil.h"
#include "dxc/DxilPIXPasses/DxilPIXPasses.h"
#include "dxc/DxilPIXPasses/DxilPIXVirtualRegisters.h"
#include "dxc/Support/Global.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ModuleSlotTracker.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

#include "PixPassHelpers.h"

#define DEBUG_TYPE "dxil-annotate-with-virtual-regs"

uint32_t CountStructMembers(llvm::Type const *pType) {
  uint32_t Count = 0;
  if (auto *VT = llvm::dyn_cast<llvm::VectorType>(pType)) {
    // Vector types can only contain scalars:
    Count = VT->getVectorNumElements();
  } else if (auto *ST = llvm::dyn_cast<llvm::StructType>(pType)) {
    for (auto &El : ST->elements()) {
      Count += CountStructMembers(El);
    }
  } else if (auto *AT = llvm::dyn_cast<llvm::ArrayType>(pType)) {
    Count = CountStructMembers(AT->getArrayElementType()) *
            AT->getArrayNumElements();
  } else {
    Count = 1;
  }
  return Count;
}

namespace {
using namespace pix_dxil;

static bool IsInstrumentableFundamentalType(llvm::Type *pAllocaTy) {
  return pAllocaTy->isFloatingPointTy() || pAllocaTy->isIntegerTy();
}

class DxilAnnotateWithVirtualRegister : public llvm::ModulePass {
public:
  static char ID;
  DxilAnnotateWithVirtualRegister() : llvm::ModulePass(ID) {}

  bool runOnModule(llvm::Module &M) override;
  void applyOptions(llvm::PassOptions O) override;

private:
  void AnnotateValues(llvm::Instruction *pI);
  void AnnotateStore(hlsl::OP *HlslOP, llvm::Instruction *pI);
  void SplitVectorStores(llvm::Instruction *pI);
  bool IsAllocaRegisterWrite(llvm::Value *V, llvm::AllocaInst **pAI,
                             llvm::Value **pIdx);
  void AnnotateAlloca(llvm::AllocaInst *pAlloca);
  void AnnotateGeneric(llvm::Instruction *pI);
  void AssignNewDxilRegister(llvm::Instruction *pI);
  void AssignNewAllocaRegister(llvm::AllocaInst *pAlloca, std::uint32_t C);
  llvm::Value *AddConstIntValues(llvm::Value *l, llvm::Value *r);
  llvm::Value *MultiplyConstIntValue(llvm::Value *l, uint32_t r);
  llvm::Value *GetStructOffset(llvm::GetElementPtrInst *pGEP,
                               uint32_t &GEPOperandIndex,
                               llvm::Type *pElementType);
  hlsl::DxilModule *m_DM;
  std::uint32_t m_uVReg;
  std::unique_ptr<llvm::ModuleSlotTracker> m_MST;
  int m_StartInstruction = 0;
  struct RememberedAllocaStores {
    llvm::StoreInst *StoreInst;
    llvm::Value *Index;
    llvm::MDNode *AllocaReg;
  };
  std::vector<RememberedAllocaStores> m_RememberedAllocaStores;

  void Init(llvm::Module &M) {
    m_DM = &M.GetOrCreateDxilModule();
    m_uVReg = 0;
    m_MST.reset(new llvm::ModuleSlotTracker(&M));
    auto functions = m_DM->GetExportedFunctions();
    for (auto &fn : functions) {
      m_MST->incorporateFunction(*fn);
    }
  }
};

void DxilAnnotateWithVirtualRegister::applyOptions(llvm::PassOptions O) {
  GetPassOptionInt(O, "startInstruction", &m_StartInstruction, 0);
}

char DxilAnnotateWithVirtualRegister::ID = 0;

static llvm::StringRef
PrintableSubsetOfMangledFunctionName(llvm::StringRef mangled) {
  llvm::StringRef printableNameSubset = mangled;
  if (mangled.size() > 2 && mangled[0] == '\1' && mangled[1] == '?') {
    printableNameSubset =
        llvm::StringRef(mangled.data() + 2, mangled.size() - 2);
  }
  return printableNameSubset;
}

bool DxilAnnotateWithVirtualRegister::runOnModule(llvm::Module &M) {
  Init(M);
  if (m_DM == nullptr) {
    return false;
  }
  unsigned int Major = 0;
  unsigned int Minor = 0;
  m_DM->GetValidatorVersion(Major, Minor);
  if (hlsl::DXIL::CompareVersions(Major, Minor, 1, 4) < 0) {
    m_DM->SetValidatorVersion(1, 4);
  }

  auto instrumentableFunctions =
      PIXPassHelpers::GetAllInstrumentableFunctions(*m_DM);

  for (auto *F : instrumentableFunctions) {
    for (auto &block : F->getBasicBlockList()) {
      for (auto it = block.begin(); it != block.end();) {
        llvm::Instruction *I = &*(it++);
        SplitVectorStores(I);
      }
    }
  }

  for (auto *F : instrumentableFunctions) {
    for (auto &block : F->getBasicBlockList()) {
      for (llvm::Instruction &I : block.getInstList()) {
        AnnotateValues(&I);
      }
    }
  }

  // Process all allocas referenced by dbg.declare intrinsics
  for (auto *F : instrumentableFunctions) {
    for (auto &block : F->getBasicBlockList()) {
      for (auto &I : block) {
        if (auto *DbgDeclare = llvm::dyn_cast<llvm::DbgDeclareInst>(&I)) {
          // The first operand of DbgDeclare is the address (typically an
          // AllocaInst)
          if (auto *AddrVal =
                  llvm::dyn_cast<llvm::Instruction>(DbgDeclare->getAddress())) {
            AnnotateValues(AddrVal);
          }
        }
      }
    }
  }

  for (auto *F : instrumentableFunctions)
    for (auto &block : F->getBasicBlockList()) {
      for (llvm::Instruction &I : block.getInstList()) {
        AnnotateStore(m_DM->GetOP(), &I);
      }
    }

  for (auto *F : instrumentableFunctions) {
    int InstructionRangeStart = m_StartInstruction;
    int InstructionRangeEnd = m_StartInstruction;
    for (auto &block : F->getBasicBlockList()) {
      for (llvm::Instruction &I : block.getInstList()) {
        // If the instruction is part of the debug value instrumentation added
        // by this pass, it doesn't need to be instrumented for the PIX user.
        uint32_t unused1, unused2;
        if (auto *Alloca = llvm::dyn_cast<llvm::AllocaInst>(&I))
          if (PixAllocaReg::FromInst(Alloca, &unused1, &unused2))
            continue;
        if (!llvm::isa<llvm::DbgDeclareInst>(&I)) {
          pix_dxil::PixDxilInstNum::AddMD(M.getContext(), &I,
                                          m_StartInstruction++);
          InstructionRangeEnd = m_StartInstruction;
        }
      }
    }
    if (OSOverride != nullptr) {
      auto shaderKind = PIXPassHelpers::GetFunctionShaderKind(*m_DM, F);
      std::string FunctioNamePlusKind =
          F->getName().str() + " " + hlsl::ShaderModel::GetKindName(shaderKind);
      *OSOverride << "InstructionRange: ";
      llvm::StringRef printableNameSubset =
          PrintableSubsetOfMangledFunctionName(FunctioNamePlusKind);
      *OSOverride << InstructionRangeStart << " " << InstructionRangeEnd << " "
                  << printableNameSubset << "\n";
    }
  }

  for (auto const &as : m_RememberedAllocaStores) {
    PixAllocaRegWrite::AddMD(m_DM->GetCtx(), as.StoreInst, as.AllocaReg,
                             as.Index);
  }

  if (OSOverride != nullptr) {
    // Print a set of strings of the exemplary form "InstructionCount: <n>
    // <fnName>"
    if (m_DM->GetShaderModel()->GetKind() == hlsl::ShaderModel::Kind::Library)
      *OSOverride << "\nIsLibrary\n";
    *OSOverride << "\nInstructionCount:" << m_StartInstruction << "\n";
  }

  m_DM = nullptr;
  return m_uVReg > 0;
}

void DxilAnnotateWithVirtualRegister::AnnotateValues(llvm::Instruction *pI) {
  if (auto *pAlloca = llvm::dyn_cast<llvm::AllocaInst>(pI)) {
    AnnotateAlloca(pAlloca);
  } else if (!pI->getType()->isPointerTy()) {
    AnnotateGeneric(pI);
  } else if (!pI->getType()->isVoidTy()) {
    AnnotateGeneric(pI);
  }
}

void DxilAnnotateWithVirtualRegister::AnnotateStore(hlsl::OP *HlslOP,
                                                    llvm::Instruction *pI) {
  auto *pSt = llvm::dyn_cast<llvm::StoreInst>(pI);
  if (pSt == nullptr) {
    return;
  }

  llvm::AllocaInst *Alloca;
  llvm::Value *Index;
  if (!IsAllocaRegisterWrite(pSt->getPointerOperand(), &Alloca, &Index)) {
    return;
  }

  llvm::MDNode *AllocaReg = Alloca->getMetadata(PixAllocaReg::MDName);
  if (AllocaReg == nullptr) {
    return;
  }
  m_RememberedAllocaStores.push_back({pSt, Index, AllocaReg});
}

llvm::Value *
DxilAnnotateWithVirtualRegister::MultiplyConstIntValue(llvm::Value *l,
                                                       uint32_t r) {
  if (r == 1)
    return l;
  if (auto *lci = llvm::dyn_cast<llvm::ConstantInt>(l))
    return m_DM->GetOP()->GetU32Const(lci->getLimitedValue() * r);
  // Should never get here, but if we do, return the left as a reasonable
  // default:
  return l;
}

llvm::Value *
DxilAnnotateWithVirtualRegister::AddConstIntValues(llvm::Value *l,
                                                   llvm::Value *r) {
  auto *rci = llvm::dyn_cast<llvm::ConstantInt>(r);
  if (rci && rci->getLimitedValue() == 0)
    return l;
  auto *lci = llvm::dyn_cast<llvm::ConstantInt>(l);
  if (lci && lci->getLimitedValue() == 0)
    return r;
  // Both an assert and a check, in case of unexpected circumstances.
  DXASSERT(lci != nullptr && rci != nullptr,
           "Both sides of add should be constant ints");
  if (lci != nullptr && rci != nullptr)
    return m_DM->GetOP()->GetU32Const(lci->getLimitedValue() +
                                      rci->getLimitedValue());
  // In an emergency, return the left argument. It'll be closest to
  // the desired value.
  return l;
}

llvm::Value *
DxilAnnotateWithVirtualRegister::GetStructOffset(llvm::GetElementPtrInst *pGEP,
                                                 uint32_t &GEPOperandIndex,
                                                 llvm::Type *pElementType) {
  if (IsInstrumentableFundamentalType(pElementType)) {
    return m_DM->GetOP()->GetU32Const(0);
  } else if (auto *pArray = llvm::dyn_cast<llvm::ArrayType>(pElementType)) {
    // 1D-array example:
    //
    // When referring to the zeroth member of the array in this struct:
    // struct smallPayload {
    //   uint32_t Array[2];
    // };
    // getelementptr inbounds% struct.smallPayload, % struct.smallPayload*% p,
    // i32 0, i32 0, i32 0 The zeros above are:
    //  -The zeroth element in the array pointed to (so, the actual struct)
    //  -The zeroth element in the struct (which is the array)
    //  -The zeroth element in that array

    auto *pArrayIndex = pGEP->getOperand(GEPOperandIndex++);

    auto pArrayElementType = pArray->getArrayElementType();
    auto *MemberIndex = MultiplyConstIntValue(
        pArrayIndex, CountStructMembers(pArrayElementType));
    return AddConstIntValues(
        MemberIndex, GetStructOffset(pGEP, GEPOperandIndex, pArrayElementType));
  } else if (auto *pStruct = llvm::dyn_cast<llvm::StructType>(pElementType)) {
    DXASSERT(GEPOperandIndex < pGEP->getNumOperands(),
             "Unexpectedly read too many GetElementPtrInst operands");

    auto *pMemberIndex =
        llvm::dyn_cast<llvm::ConstantInt>(pGEP->getOperand(GEPOperandIndex++));

    if (pMemberIndex == nullptr) {
      return m_DM->GetOP()->GetU32Const(0);
    }

    uint32_t MemberIndex = pMemberIndex->getLimitedValue();

    uint32_t MemberOffset = 0;
    for (uint32_t i = 0; i < MemberIndex; ++i) {
      MemberOffset += CountStructMembers(pStruct->getElementType(i));
    }

    return AddConstIntValues(
        m_DM->GetOP()->GetU32Const(MemberOffset),
        GetStructOffset(pGEP, GEPOperandIndex,
                        pStruct->getElementType(MemberIndex)));
  } else {
    return m_DM->GetOP()->GetU32Const(0);
  }
}

bool DxilAnnotateWithVirtualRegister::IsAllocaRegisterWrite(
    llvm::Value *V, llvm::AllocaInst **pAI, llvm::Value **pIdx) {

  *pAI = nullptr;
  *pIdx = nullptr;

  if (auto *pGEP = llvm::dyn_cast<llvm::GetElementPtrInst>(V)) {
    uint32_t precedingMemberCount = 0;
    auto *Alloca = llvm::dyn_cast<llvm::AllocaInst>(pGEP->getPointerOperand());
    if (Alloca == nullptr) {
      // In the case of vector types (floatN, matrixNxM), the pointer operand
      // will actually point to another element pointer instruction. But this
      // isn't a recursive thing- we only need to check these two levels.
      if (auto *pPointerGEP = llvm::dyn_cast<llvm::GetElementPtrInst>(
              pGEP->getPointerOperand())) {
        Alloca =
            llvm::dyn_cast<llvm::AllocaInst>(pPointerGEP->getPointerOperand());
        if (Alloca == nullptr) {
          return false;
        }
        // And of course the member we're after might not be at the beginning of
        // any containing struct:
        if (auto *pStructType = llvm::dyn_cast<llvm::StructType>(
                pPointerGEP->getPointerOperandType()
                    ->getPointerElementType())) {
          auto *pStructMember =
              llvm::dyn_cast<llvm::ConstantInt>(pPointerGEP->getOperand(2));
          uint64_t memberIndex = pStructMember->getLimitedValue();
          for (uint64_t i = 0; i < memberIndex; ++i) {
            precedingMemberCount +=
                CountStructMembers(pStructType->getStructElementType(i));
          }
        }

        // And the source pointer may be a vector (floatn) type,
        // and if so, that's another offset to consider.
        llvm::Type *DestType = pGEP->getPointerOperand()->getType();
        // We expect this to be a pointer type (it's a GEP after all):
        if (DestType->isPointerTy()) {
          llvm::Type *PointedType = DestType->getPointerElementType();
          // Being careful to check num operands too in order to avoid false
          // positives:
          if (PointedType->isVectorTy() && pGEP->getNumOperands() == 3) {
            // Fetch the second deref (in operand 2).
            // (the first derefs the pointer to the "floatn",
            // and the second denotes the index into the floatn.)
            llvm::Value *vectorIndex = pGEP->getOperand(2);
            if (auto *constIntIIndex =
                    llvm::cast<llvm::ConstantInt>(vectorIndex)) {
              precedingMemberCount += constIntIIndex->getLimitedValue();
            }
          }
        }
      } else {
        return false;
      }
    }

    // Deref pointer type to get struct type:
    llvm::Type *pStructType = pGEP->getPointerOperandType();
    pStructType = pStructType->getContainedType(0);

    // The 1th operand is an index into the array of the above type. In DXIL
    // derived from HLSL, we always expect this to be 0 (since llvm structs are
    // only used for single-valued objects in HLSL, such as the
    // amplification-to-mesh or TraceRays payloads).
    uint32_t GEPOperandIndex = 1;
    auto *pBaseArrayIndex =
        llvm::dyn_cast<llvm::ConstantInt>(pGEP->getOperand(GEPOperandIndex++));
    DXASSERT_LOCALVAR(pBaseArrayIndex, pBaseArrayIndex != nullptr,
                      "null base array index pointer");
    DXASSERT_LOCALVAR(pBaseArrayIndex, pBaseArrayIndex->getLimitedValue() == 0,
                      "unexpected >0 array index");

    // From here on, the indices always come in groups: first, the type
    // referenced in the current struct. If that type is an (n-dimensional)
    // array, then there follow n indices.

    auto offset = GetStructOffset(pGEP, GEPOperandIndex, pStructType);

    llvm::Value *IndexValue = AddConstIntValues(
        offset, m_DM->GetOP()->GetU32Const(precedingMemberCount));

    if (IndexValue != nullptr) {
      *pAI = Alloca;
      *pIdx = IndexValue;
      return true;
    }
    return false;
  }

  if (auto *pAlloca = llvm::dyn_cast<llvm::AllocaInst>(V)) {
    llvm::Type *pAllocaTy = pAlloca->getType()->getElementType();
    if (!IsInstrumentableFundamentalType(pAllocaTy)) {
      return false;
    }

    *pAI = pAlloca;
    *pIdx = m_DM->GetOP()->GetU32Const(0);
    return true;
  }

  return false;
}

void DxilAnnotateWithVirtualRegister::AnnotateAlloca(
    llvm::AllocaInst *pAlloca) {
  llvm::Type *pAllocaTy = pAlloca->getType()->getElementType();
  if (IsInstrumentableFundamentalType(pAllocaTy)) {
    AssignNewAllocaRegister(pAlloca, 1);
  } else if (auto *AT = llvm::dyn_cast<llvm::ArrayType>(pAllocaTy)) {
    AssignNewAllocaRegister(pAlloca, AT->getNumElements());
  } else if (auto *VT = llvm::dyn_cast<llvm::VectorType>(pAllocaTy)) {
    AssignNewAllocaRegister(pAlloca, VT->getNumElements());
  } else if (auto *ST = llvm::dyn_cast<llvm::StructType>(pAllocaTy)) {
    AssignNewAllocaRegister(pAlloca, CountStructMembers(ST));
  } else {
    DXASSERT_ARGS(false, "Unhandled alloca kind: %d", pAllocaTy->getTypeID());
  }
}

void DxilAnnotateWithVirtualRegister::AnnotateGeneric(llvm::Instruction *pI) {
  if (auto *GEP = llvm::dyn_cast<llvm::GetElementPtrInst>(pI)) {
    // https://llvm.org/docs/LangRef.html#getelementptr-instruction
    DXASSERT(!GEP->getOperand(1)->getType()->isVectorTy(),
             "struct vectors not supported");
    llvm::AllocaInst *StructAlloca =
        llvm::dyn_cast<llvm::AllocaInst>(GEP->getOperand(0));
    if (StructAlloca != nullptr) {
      // This is the case of a pointer to a struct member.
      // We treat it as an alias of the actual member in the alloca.
      std::uint32_t baseStructRegNum = 0;
      std::uint32_t regSize = 0;
      if (pix_dxil::PixAllocaReg::FromInst(StructAlloca, &baseStructRegNum,
                                           &regSize)) {
        llvm::ConstantInt *OffsetAsInt =
            llvm::dyn_cast<llvm::ConstantInt>(GEP->getOperand(2));
        if (OffsetAsInt != nullptr) {
          std::uint32_t OffsetInElementsFromStructureStart =
              static_cast<std::uint32_t>(
                  OffsetAsInt->getValue().getLimitedValue());
          DXASSERT(OffsetInElementsFromStructureStart < regSize,
                   "Structure member offset out of expected range");
          std::uint32_t OffsetInValuesFromStructureStart =
              OffsetInElementsFromStructureStart;
          if (auto *ST = llvm::dyn_cast<llvm::StructType>(
                  GEP->getPointerOperandType()->getPointerElementType())) {
            DXASSERT(OffsetInElementsFromStructureStart < ST->getNumElements(),
                     "Offset into struct is bigger than struct");
            OffsetInValuesFromStructureStart = 0;
            for (std::uint32_t Element = 0;
                 Element < OffsetInElementsFromStructureStart; ++Element) {
              OffsetInValuesFromStructureStart +=
                  CountStructMembers(ST->getElementType(Element));
            }
          }
          PixDxilReg::AddMD(m_DM->GetCtx(), pI,
                            baseStructRegNum +
                                OffsetInValuesFromStructureStart);
        }
      }
    }
  } else {
    if (!IsInstrumentableFundamentalType(pI->getType())) {
      return;
    }
    AssignNewDxilRegister(pI);
  }
}

void DxilAnnotateWithVirtualRegister::AssignNewDxilRegister(
    llvm::Instruction *pI) {
  PixDxilReg::AddMD(m_DM->GetCtx(), pI, m_uVReg);
  m_uVReg++;
}

void DxilAnnotateWithVirtualRegister::AssignNewAllocaRegister(
    llvm::AllocaInst *pAlloca, std::uint32_t C) {
  if (!PixAllocaReg::FromInst(pAlloca, nullptr, nullptr)) {
    PixAllocaReg::AddMD(m_DM->GetCtx(), pAlloca, m_uVReg, C);
    m_uVReg += C;
  }
}

void DxilAnnotateWithVirtualRegister::SplitVectorStores(llvm::Instruction *pI) {
  auto *pSt = llvm::dyn_cast<llvm::StoreInst>(pI);
  if (pSt == nullptr) {
    return;
  }

  llvm::AllocaInst *Alloca;
  llvm::Value *Index;
  if (!IsAllocaRegisterWrite(pSt->getPointerOperand(), &Alloca, &Index)) {
    return;
  }

  llvm::Type *SourceType = pSt->getValueOperand()->getType();
  if (SourceType->isVectorTy()) {
    if (auto *constIntIIndex = llvm::cast<llvm::ConstantInt>(Index)) {
      // break vector alloca stores up into individual stores
      llvm::IRBuilder<> B(pSt);
      for (uint64_t el = 0; el < SourceType->getVectorNumElements(); ++el) {
        llvm::Value *destPointer = B.CreateGEP(pSt->getPointerOperand(),
                                               {B.getInt32(0), B.getInt32(el)});
        llvm::Value *source =
            B.CreateExtractElement(pSt->getValueOperand(), el);
        B.CreateStore(source, destPointer);
      }
      pI->eraseFromParent();
    }
  }
}

} // namespace

using namespace llvm;

INITIALIZE_PASS(DxilAnnotateWithVirtualRegister, DEBUG_TYPE,
                "Annotates each instruction in the DXIL module with a virtual "
                "register number",
                false, false)

ModulePass *llvm::createDxilAnnotateWithVirtualRegisterPass() {
  return new DxilAnnotateWithVirtualRegister();
}
