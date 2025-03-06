///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilCleanup.cpp                                                           //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Optimization of DXIL after conversion from DXBC.                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

//===----------------------------------------------------------------------===//
//                    DXIL Cleanup Transformation
//===----------------------------------------------------------------------===//
//
// The pass cleans up DXIL obtained after conversion from DXBC.
// Essentially, the pass construct efficient SSA for DXBC r-registers and
// performs the following:
//   1. Removes TempRegStore/TempRegLoad calls, replacing DXBC registers with
//      either temporary or global LLVM values.
//   2. Minimizes the number of bitcasts induced by the lack of types in DXBC.
//   3. Removes helper operations to support DXBC conditionals, translated to
//   i1.
//   4. Recovers doubles from pairs of 32-bit DXBC registers.
//   5. Removes MinPrecXRegLoad and MinPrecXRegStore for DXBC indexable,
//      min-presicion x-registers.
//
// Clarification of important algorithmic decisions:
//   1. A live range (LR) is all defs connected via phi-nodes. A straightforward
//      recursive algorithm is used to collect LR's set of defs.
//   2. Live ranges are "connected" to other liver ranges via DXIL bitcasts.
//      This creates a bitcast graphs.
//   3. Live ranges are assigned types based on the number of float (F) or
//      integer (I) defs. A bitcast def initially has an unknow type (U).
//      Each LR is assigned type only once. LRs are processed in dynamic order
//      biased towards LRs with known types, e.g., numF > numI + numU.
//      When a LR is assigned final type, emanating bitcasts become "resolved"
//      and contribute desired type to the neighboring LRs.
//   4. After all LRs are processed, each LR is assigned final type based on
//      the number of F and I defs. If type changed from the initial assumption,
//      the code is rewritten accordingly: new bitcasts are inserted for
//      correctness.
//   5. After every LR type is finalized, chains of bitcasts are cleaned up.
//   6. The algorithm splits 16- and 32-bit LRs.
//   7. Registers that are used in an entry and another subroutine are
//      represented as global variables.
//

#include "DxilConvPasses/DxilCleanup.h"
#include "dxc/DXIL/DxilInstructions.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/Support/Global.h"

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/MapVector.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Scalar.h"

#include <algorithm>
#include <queue>
#include <set>
#include <utility>
#include <vector>

using namespace llvm;
using namespace llvm::legacy;
using namespace hlsl;
using std::pair;
using std::set;
using std::string;
using std::vector;

#define DXILCLEANUP_DBG 0

#define DEBUG_TYPE "dxilcleanup"

#if DXILCLEANUP_DBG
static void debugprint(const char *banner, Module &M) {
  std::string buf;
  raw_string_ostream os(buf);
  os << banner << "\n";
  M.print(os, nullptr);
  os.flush();
  std::puts(buf.c_str());
}
#endif

namespace DxilCleanupNS {

/// Use this class to optimize DXIL after conversion from DXBC.
class DxilCleanup : public ModulePass {
public:
  static char ID;

  DxilCleanup() : ModulePass(ID), m_pCtx(nullptr), m_pModule(nullptr) {
    initializeDxilCleanupPass(*PassRegistry::getPassRegistry());
  }

  virtual bool runOnModule(Module &M);

  struct LiveRange {
    unsigned id;
    SmallVector<Value *, 4> defs;
    SmallDenseMap<unsigned, unsigned, 4> bitcastMap;
    unsigned numI;
    unsigned numF;
    unsigned numU;
    Type *pNewType;
    LiveRange() : id(0), numI(0), numF(0), numU(0), pNewType(nullptr) {}
    LiveRange operator=(const LiveRange &) = delete;

    // I cannot delete these constructors, because vector depends on them, even
    // if I never trigger them. So assert if they are hit instead.
    LiveRange(const LiveRange &other)
        : id(other.id), defs(other.defs), bitcastMap(other.bitcastMap),
          numI(other.numI), numF(other.numF), numU(other.numU),
          pNewType(other.pNewType) {
      DXASSERT_NOMSG(false);
    }
    LiveRange(LiveRange &&other)
        : id(other.id), defs(std::move(other.defs)),
          bitcastMap(std::move(other.bitcastMap)), numI(other.numI),
          numF(other.numF), numU(other.numU), pNewType(other.pNewType) {
      DXASSERT_NOMSG(false);
    }

    unsigned GetCaseNumber() const;
    void GuessType(LLVMContext &Ctx);
    bool operator<(const LiveRange &other) const;
  };

private:
  const unsigned kRegCompAlignment = 4;

  LLVMContext *m_pCtx;
  Module *m_pModule;
  DxilModule *m_pDxilModule;

  vector<LiveRange> m_LiveRanges;
  DenseMap<Value *, unsigned> m_LiveRangeMap;

  void OptimizeIdxRegDecls();
  bool OptimizeIdxRegDecls_CollectUsage(Value *pDecl, unsigned &numF,
                                        unsigned &numI);
  bool OptimizeIdxRegDecls_CollectUsageForUser(User *U, bool bFlt, bool bInt,
                                               unsigned &numF, unsigned &numI);
  Type *OptimizeIdxRegDecls_DeclareType(Type *pOldType);
  void OptimizeIdxRegDecls_ReplaceDecl(Value *pOldDecl, Value *pNewDecl,
                                       vector<Instruction *> &InstrToErase);
  void OptimizeIdxRegDecls_ReplaceGEPUse(Value *pOldGEPUser, Value *pNewGEP,
                                         Value *pOldDecl, Value *pNewDecl,
                                         vector<Instruction *> &InstrToErase);

  void RemoveRegLoadStore();
  void ConstructSSA();
  void CollectLiveRanges();
  void CountLiveRangeRec(unsigned LRId, Instruction *pInst);
  void RecoverLiveRangeRec(LiveRange &LR, Instruction *pInst);
  void InferLiveRangeTypes();
  void ChangeLiveRangeTypes();
  void CleanupPatterns();
  void RemoveDeadCode();

  Value *CastValue(Value *pValue, Type *pToType, Instruction *pOrigInst);
  bool IsDxilBitcast(Value *pValue);
  ArrayType *GetDeclArrayType(Type *pSrcType);
  Type *GetDeclScalarType(Type *pSrcType);
};

char DxilCleanup::ID = 0;

//------------------------------------------------------------------------------
//
//  DxilCleanup methods.
//
bool DxilCleanup::runOnModule(Module &M) {
  m_pModule = &M;
  m_pCtx = &M.getContext();
  m_pDxilModule = &m_pModule->GetOrCreateDxilModule();

  OptimizeIdxRegDecls();
  RemoveRegLoadStore();
  ConstructSSA();
  CollectLiveRanges();
  InferLiveRangeTypes();
  ChangeLiveRangeTypes();
  CleanupPatterns();
  RemoveDeadCode();

  return true;
}

void DxilCleanup::OptimizeIdxRegDecls() {
  // 1. Convert global x-register decl into alloca if used only in one function.
  for (auto itGV = m_pModule->global_begin(), endGV = m_pModule->global_end();
       itGV != endGV;) {
    GlobalVariable *GV = itGV;
    ++itGV;
    if (GV->isConstant() || GV->getLinkage() != GlobalValue::InternalLinkage)
      continue;
    PointerType *pPtrType = dyn_cast<PointerType>(GV->getType());
    if (!pPtrType || pPtrType->getAddressSpace() != DXIL::kDefaultAddrSpace)
      continue;

    Type *pElemType = pPtrType->getElementType();

    Function *F = nullptr;
    for (User *U : GV->users()) {
      Instruction *I = dyn_cast<Instruction>(U);
      if (!I || (F && I->getParent()->getParent() != F)) {
        F = nullptr;
        break;
      }

      F = cast<Function>(I->getParent()->getParent());
    }

    if (F) {
      // Promote to alloca.
      Instruction *pAnchor = F->getEntryBlock().begin();
      AllocaInst *AI =
          new AllocaInst(pElemType, nullptr, GV->getName(), pAnchor);
      AI->setAlignment(GV->getAlignment());
      GV->replaceAllUsesWith(AI);
      GV->eraseFromParent();
    }
  }

  // 2. Collect x-register alloca usage stats and change type, if profitable.
  for (auto itF = m_pModule->begin(), endFn = m_pModule->end(); itF != endFn;
       ++itF) {
    Function *F = itF;
    if (F->empty())
      continue;
    BasicBlock *pEntryBB = &F->getEntryBlock();
    vector<Instruction *> InstrToErase;

    for (auto itInst = pEntryBB->begin(), endInst = pEntryBB->end();
         itInst != endInst; ++itInst) {
      AllocaInst *AI = dyn_cast<AllocaInst>(itInst);
      if (!AI)
        continue;

      Type *pScalarType = GetDeclScalarType(AI->getType());
      if (pScalarType != Type::getFloatTy(*m_pCtx) &&
          pScalarType != Type::getHalfTy(*m_pCtx) &&
          pScalarType != Type::getInt32Ty(*m_pCtx) &&
          pScalarType != Type::getInt16Ty(*m_pCtx)) {
        continue;
      }

      // Collect usage stats and potentially change decl type.
      unsigned numF, numI;
      if (OptimizeIdxRegDecls_CollectUsage(AI, numF, numI)) {
        Type *pScalarType = GetDeclScalarType(AI->getType());
        if ((pScalarType->isFloatingPointTy() && numI > numF) ||
            (pScalarType->isIntegerTy() && numF >= numI)) {
          Type *pNewType = OptimizeIdxRegDecls_DeclareType(AI->getType());
          if (pNewType) {
            // Replace alloca.
            AllocaInst *AI2 =
                new AllocaInst(pNewType, nullptr, AI->getName(), AI);
            AI2->setAlignment(AI->getAlignment());
            OptimizeIdxRegDecls_ReplaceDecl(AI, AI2, InstrToErase);
            InstrToErase.emplace_back(AI);
          }
        }
      }
    }

    for (auto *I : InstrToErase) {
      I->eraseFromParent();
    }
  }

  // 3. Collect x-register global decl usage stats and change type, if
  // profitable.
  llvm::SmallVector<GlobalVariable *, 4> GVWorklist;
  for (auto itGV = m_pModule->global_begin(), endGV = m_pModule->global_end();
       itGV != endGV;) {
    GlobalVariable *pOldGV = itGV;
    ++itGV;
    if (pOldGV->isConstant())
      continue;
    PointerType *pOldPtrType = dyn_cast<PointerType>(pOldGV->getType());
    if (!pOldPtrType ||
        pOldPtrType->getAddressSpace() != DXIL::kDefaultAddrSpace)
      continue;

    unsigned numF, numI;
    if (OptimizeIdxRegDecls_CollectUsage(pOldGV, numF, numI)) {
      Type *pScalarType = GetDeclScalarType(pOldGV->getType());
      if ((pScalarType->isFloatingPointTy() && numI > numF) ||
          (pScalarType->isIntegerTy() && numF >= numI)) {
        GVWorklist.push_back(pOldGV);
      }
    }
  }

  for (auto pOldGV : GVWorklist) {
    if (Type *pNewType = OptimizeIdxRegDecls_DeclareType(pOldGV->getType())) {
      // Replace global decl.
      PointerType *pOldPtrType = dyn_cast<PointerType>(pOldGV->getType());
      GlobalVariable *pNewGV = new GlobalVariable(
          *m_pModule, pNewType, false, pOldGV->getLinkage(),
          UndefValue::get(pNewType), pOldGV->getName(), nullptr,
          pOldGV->getThreadLocalMode(), pOldPtrType->getAddressSpace());
      vector<Instruction *> InstrToErase;
      OptimizeIdxRegDecls_ReplaceDecl(pOldGV, pNewGV, InstrToErase);
      for (auto *I : InstrToErase) {
        I->eraseFromParent();
      }
      pOldGV->eraseFromParent();
    }
  }
}

ArrayType *DxilCleanup::GetDeclArrayType(Type *pSrcType) {
  PointerType *pPtrType = dyn_cast<PointerType>(pSrcType);
  if (!pPtrType)
    return nullptr;

  if (ArrayType *pArrayType = dyn_cast<ArrayType>(pPtrType->getElementType())) {
    return pArrayType;
  }

  return nullptr;
}

Type *DxilCleanup::GetDeclScalarType(Type *pSrcType) {
  PointerType *pPtrType = dyn_cast<PointerType>(pSrcType);
  if (!pPtrType)
    return nullptr;

  Type *pScalarType = pPtrType->getElementType();
  if (ArrayType *pArrayType = dyn_cast<ArrayType>(pScalarType)) {
    pScalarType = pArrayType->getArrayElementType();
  }

  return pScalarType;
}

Type *DxilCleanup::OptimizeIdxRegDecls_DeclareType(Type *pOldType) {
  Type *pNewType = nullptr;
  Type *pScalarType = GetDeclScalarType(pOldType);
  if (ArrayType *pArrayType = GetDeclArrayType(pOldType)) {
    uint64_t ArraySize = pArrayType->getArrayNumElements();
    if (pScalarType == Type::getFloatTy(*m_pCtx)) {
      pNewType = ArrayType::get(Type::getInt32Ty(*m_pCtx), ArraySize);
    } else if (pScalarType == Type::getHalfTy(*m_pCtx)) {
      pNewType = ArrayType::get(Type::getInt16Ty(*m_pCtx), ArraySize);
    } else if (pScalarType == Type::getInt32Ty(*m_pCtx)) {
      pNewType = ArrayType::get(Type::getFloatTy(*m_pCtx), ArraySize);
    } else if (pScalarType == Type::getInt16Ty(*m_pCtx)) {
      pNewType = ArrayType::get(Type::getHalfTy(*m_pCtx), ArraySize);
    } else {
      IFT(DXC_E_OPTIMIZATION_FAILED);
    }
  } else {
    if (pScalarType == Type::getFloatTy(*m_pCtx)) {
      pNewType = Type::getInt32Ty(*m_pCtx);
    } else if (pScalarType == Type::getHalfTy(*m_pCtx)) {
      pNewType = Type::getInt16Ty(*m_pCtx);
    } else if (pScalarType == Type::getInt32Ty(*m_pCtx)) {
      pNewType = Type::getFloatTy(*m_pCtx);
    } else if (pScalarType == Type::getInt16Ty(*m_pCtx)) {
      pNewType = Type::getHalfTy(*m_pCtx);
    } else {
      IFT(DXC_E_OPTIMIZATION_FAILED);
    }
  }
  return pNewType;
}

bool DxilCleanup::OptimizeIdxRegDecls_CollectUsage(Value *pDecl, unsigned &numF,
                                                   unsigned &numI) {
  numF = numI = 0;
  Type *pScalarType = GetDeclScalarType(pDecl->getType());
  if (!pScalarType)
    return false;
  bool bFlt = pScalarType == Type::getFloatTy(*m_pCtx) ||
              pScalarType == Type::getHalfTy(*m_pCtx);
  bool bInt = pScalarType == Type::getInt32Ty(*m_pCtx) ||
              pScalarType == Type::getInt16Ty(*m_pCtx);
  if (!(bFlt || bInt))
    return false;

  for (User *U : pDecl->users()) {
    if (GetElementPtrInst *pGEP = dyn_cast<GetElementPtrInst>(U)) {
      for (User *U2 : pGEP->users()) {
        if (!OptimizeIdxRegDecls_CollectUsageForUser(U2, bFlt, bInt, numF,
                                                     numI))
          return false;
      }
    } else if (GEPOperator *pGEP = dyn_cast<GEPOperator>(U)) {
      for (User *U2 : pGEP->users()) {
        if (!OptimizeIdxRegDecls_CollectUsageForUser(U2, bFlt, bInt, numF,
                                                     numI))
          return false;
      }
    } else if (BitCastInst *pBC = dyn_cast<BitCastInst>(U)) {
      if (pBC->getType() != Type::getDoublePtrTy(*m_pCtx))
        return false;
    } else {
      return false;
    }
  }

  return true;
}

bool DxilCleanup::OptimizeIdxRegDecls_CollectUsageForUser(User *U, bool bFlt,
                                                          bool bInt,
                                                          unsigned &numF,
                                                          unsigned &numI) {
  if (LoadInst *LI = dyn_cast<LoadInst>(U)) {
    for (User *U2 : LI->users()) {
      if (!IsDxilBitcast(U2)) {
        if (bFlt)
          numF++;
        if (bInt)
          numI++;
      } else {
        if (bFlt)
          numI++;
        if (bInt)
          numF++;
      }
    }
  } else if (StoreInst *SI = dyn_cast<StoreInst>(U)) {
    Value *pValue = SI->getValueOperand();
    if (!IsDxilBitcast(pValue)) {
      if (bFlt)
        numF++;
      if (bInt)
        numI++;
    } else {
      if (bFlt)
        numI++;
      if (bInt)
        numF++;
    }
  } else {
    return false;
  }

  return true;
}

void DxilCleanup::OptimizeIdxRegDecls_ReplaceDecl(
    Value *pOldDecl, Value *pNewDecl, vector<Instruction *> &InstrToErase) {
  for (auto itU = pOldDecl->use_begin(), endU = pOldDecl->use_end();
       itU != endU; ++itU) {
    User *I = itU->getUser();

    if (GetElementPtrInst *pOldGEP = dyn_cast<GetElementPtrInst>(I)) {
      // Case 1. Load.
      //   %44 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0,
      //   i32 %43 %45 = load float, float* %44, align 4 %46 = add float %45,
      //   ...
      // becomes
      //   %44 = getelementptr [24 x i32], [24 x i32]* %dx.v32.x0, i32 0, i32
      //   %43 %45 = load i32, i32* %44, align 4 %t1 = call float
      //   @dx.op.bitcastI32toF32 i32 %45 %46 = add i32 %t1, ...
      //
      // Case 2. Store.
      //   %31 = add float ...
      //   %32 = getelementptr [24 x float], [24 x float]* %dx.v32.x0, i32 0,
      //   i32 16 store float %31, float* %32, align 4
      // becomes
      //   %31 = add float ...
      //   %32 = getelementptr [24 x i32], [24 x i32]* %dx.v32.x0, i32 0, i32 16
      //   %t1 = call i32 @dx.op.bitcastF32toI32 float %31
      //   store i32 %t1, i32* %32, align 4
      //
      SmallVector<Value *, 4> GEPIndices;
      for (auto i = pOldGEP->idx_begin(), e = pOldGEP->idx_end(); i != e; i++) {
        GEPIndices.push_back(*i);
      }
      GetElementPtrInst *pNewGEP =
          GetElementPtrInst::Create(nullptr, pNewDecl, GEPIndices,
                                    pOldGEP->getName(), pOldGEP->getNextNode());
      for (auto itU2 = pOldGEP->use_begin(), endU2 = pOldGEP->use_end();
           itU2 != endU2; ++itU2) {
        Value *pOldGEPUser = itU2->getUser();
        OptimizeIdxRegDecls_ReplaceGEPUse(pOldGEPUser, pNewGEP, pOldDecl,
                                          pNewDecl, InstrToErase);
      }
      InstrToErase.emplace_back(pOldGEP);
    } else if (GEPOperator *pOldGEP = dyn_cast<GEPOperator>(I)) {
      // The cases are the same as for the GetElementPtrInst above.
      SmallVector<Value *, 4> GEPIndices;
      for (auto i = pOldGEP->idx_begin(), e = pOldGEP->idx_end(); i != e; i++) {
        GEPIndices.push_back(*i);
      }
      Type *pNewGEPElemType =
          cast<PointerType>(pNewDecl->getType())->getElementType();
      Constant *pNewGEPOp = ConstantExpr::getGetElementPtr(
          pNewGEPElemType, cast<Constant>(pNewDecl), GEPIndices,
          pOldGEP->isInBounds());
      GEPOperator *pNewGEP = cast<GEPOperator>(pNewGEPOp);
      for (auto itU2 = pOldGEP->use_begin(), endU2 = pOldGEP->use_end();
           itU2 != endU2; ++itU2) {
        Value *pOldGEPUser = itU2->getUser();
        OptimizeIdxRegDecls_ReplaceGEPUse(pOldGEPUser, pNewGEP, pOldDecl,
                                          pNewDecl, InstrToErase);
      }
    } else if (BitCastInst *pOldBC = dyn_cast<BitCastInst>(I)) {
      //   %1 = bitcast [24 x float]* %dx.v32.x0 to double*
      // becomes
      //   %1 = bitcast [24 x i32]* %dx.v32.x0 to double*
      BitCastInst *pNewBC =
          new BitCastInst(pNewDecl, pOldBC->getType(), pOldBC->getName(),
                          pOldBC->getNextNode());
      pOldBC->replaceAllUsesWith(pNewBC);
      InstrToErase.emplace_back(pOldBC);
    } else {
      IFT(DXC_E_OPTIMIZATION_FAILED);
    }
  }
}

void DxilCleanup::OptimizeIdxRegDecls_ReplaceGEPUse(
    Value *pOldGEPUser, Value *pNewGEP, Value *pOldDecl, Value *pNewDecl,
    vector<Instruction *> &InstrToErase) {
  if (LoadInst *pOldLI = dyn_cast<LoadInst>(pOldGEPUser)) {
    LoadInst *pNewLI =
        new LoadInst(pNewGEP, pOldLI->getName(), pOldLI->getNextNode());
    pNewLI->setAlignment(pOldLI->getAlignment());
    Value *pNewValue = CastValue(pNewLI, GetDeclScalarType(pOldDecl->getType()),
                                 pNewLI->getNextNode());
    pOldLI->replaceAllUsesWith(pNewValue);
    InstrToErase.emplace_back(pOldLI);
  } else if (StoreInst *pOldSI = dyn_cast<StoreInst>(pOldGEPUser)) {
    Value *pOldValue = pOldSI->getValueOperand();
    Value *pNewValue =
        CastValue(pOldValue, GetDeclScalarType(pNewDecl->getType()), pOldSI);
    StoreInst *pNewSI =
        new StoreInst(pNewValue, pNewGEP, pOldSI->getNextNode());
    pNewSI->setAlignment(pOldSI->getAlignment());
    InstrToErase.emplace_back(pOldSI);
  } else {
    IFT(DXC_E_OPTIMIZATION_FAILED);
  }
}

void DxilCleanup::RemoveRegLoadStore() {
  struct RegRec {
    unsigned numI32;
    unsigned numF32;
    unsigned numI16;
    unsigned numF16;
    Value *pDecl32;
    Value *pDecl16;
    RegRec()
        : numI32(0), numF32(0), numI16(0), numF16(0), pDecl32(nullptr),
          pDecl16(nullptr) {}
  };
  struct FuncRec {
    MapVector<unsigned, RegRec> RegMap;
    bool bEntry;
    bool bCallsOtherFunc;
    FuncRec() : bEntry(false), bCallsOtherFunc(false) {}
  };
  MapVector<Function *, FuncRec> FuncMap;

  // 1. For each r-register, collect usage stats.
  for (auto itF = m_pModule->begin(), endFn = m_pModule->end(); itF != endFn;
       ++itF) {
    Function *F = itF;
    if (F->empty())
      continue;
    DXASSERT_NOMSG(FuncMap.find(F) == FuncMap.end());
    FuncRec &FR = FuncMap[F];

    // Detect entry.
    if (F == m_pDxilModule->GetEntryFunction() ||
        F == m_pDxilModule->GetPatchConstantFunction()) {
      FR.bEntry = true;
    }

    for (auto itBB = F->begin(), endBB = F->end(); itBB != endBB; ++itBB) {
      BasicBlock *BB = itBB;

      for (auto itInst = BB->begin(), endInst = BB->end(); itInst != endInst;
           ++itInst) {
        CallInst *CI = dyn_cast<CallInst>(itInst);
        if (!CI)
          continue;

        if (!OP::IsDxilOpFuncCallInst(CI)) {
          FuncMap[F].bCallsOtherFunc = true;
          continue;
        }

        // Obtain register index for TempRegLoad/TempRegStore.
        unsigned regIdx = 0;
        Type *pValType = nullptr;
        if (DxilInst_TempRegLoad TRL = DxilInst_TempRegLoad(CI)) {
          regIdx = dyn_cast<ConstantInt>(TRL.get_index())->getZExtValue();
          pValType = CI->getType();
        } else if (DxilInst_TempRegStore TRS = DxilInst_TempRegStore(CI)) {
          regIdx = dyn_cast<ConstantInt>(TRS.get_index())->getZExtValue();
          pValType = TRS.get_value()->getType();
        } else {
          continue;
        }

        // Update register usage.
        RegRec &reg = FR.RegMap[regIdx];
        if (pValType == Type::getFloatTy(*m_pCtx)) {
          reg.numF32++;
        } else if (pValType == Type::getInt32Ty(*m_pCtx)) {
          reg.numI32++;
        } else if (pValType == Type::getHalfTy(*m_pCtx)) {
          reg.numF16++;
        } else if (pValType == Type::getInt16Ty(*m_pCtx)) {
          reg.numI16++;
        } else {
          IFT(DXC_E_OPTIMIZATION_FAILED);
        }
      }
    }
  }

  // 2. Declare local and global variables to represent each r-register.
  for (auto &itF : FuncMap) {
    Function *F = itF.first;
    FuncRec &FR = itF.second;

    for (auto &itReg : FR.RegMap) {
      unsigned regIdx = itReg.first;
      RegRec &reg = itReg.second;
      DXASSERT_NOMSG(reg.pDecl16 == nullptr && reg.pDecl32 == nullptr);

      enum class DeclKind { None, Alloca, Global };
      DeclKind Decl32Kind =
          (reg.numF32 + reg.numI32) == 0 ? DeclKind::None : DeclKind::Alloca;
      DeclKind Decl16Kind =
          (reg.numF16 + reg.numI16) == 0 ? DeclKind::None : DeclKind::Alloca;
      DXASSERT_NOMSG(Decl32Kind == DeclKind::Alloca ||
                     Decl16Kind == DeclKind::Alloca);
      unsigned numF32 = reg.numF32, numI32 = reg.numI32, numF16 = reg.numF16,
               numI16 = reg.numI16;
      if (!FR.bEntry || FR.bCallsOtherFunc) {
        // Check if register is used in another function.
        for (auto &itF2 : FuncMap) {
          Function *F2 = itF2.first;
          FuncRec &FR2 = itF2.second;
          if (F2 == F || (FR.bEntry && FR2.bEntry))
            continue;

          auto itReg2 = FR2.RegMap.find(regIdx);
          if (itReg2 == FR2.RegMap.end())
            continue;

          RegRec &reg2 = itReg2->second;
          if (Decl32Kind == DeclKind::Alloca &&
              (reg2.numF32 + reg2.numI32) > 0) {
            Decl32Kind = DeclKind::Global;
          }
          if (Decl16Kind == DeclKind::Alloca &&
              (reg2.numF16 + reg2.numI16) > 0) {
            Decl16Kind = DeclKind::Global;
          }
          numF32 += reg2.numF32;
          numI32 += reg2.numI32;
          numF16 += reg2.numF16;
          numI16 += reg2.numI16;
        }
      }

      // Declare variables.
      if (Decl32Kind == DeclKind::Alloca) {
        Twine regName = Twine("dx.v32.r") + Twine(regIdx);
        Type *pDeclType = numF32 >= numI32 ? Type::getFloatTy(*m_pCtx)
                                           : Type::getInt32Ty(*m_pCtx);
        Instruction *pAnchor = F->getEntryBlock().begin();
        AllocaInst *AI = new AllocaInst(pDeclType, nullptr, regName, pAnchor);
        AI->setAlignment(kRegCompAlignment);
        reg.pDecl32 = AI;
      }
      if (Decl16Kind == DeclKind::Alloca) {
        Twine regName = Twine("dx.v16.r") + Twine(regIdx);
        Type *pDeclType = numF16 >= numI16 ? Type::getHalfTy(*m_pCtx)
                                           : Type::getInt16Ty(*m_pCtx);
        Instruction *pAnchor = F->getEntryBlock().begin();
        AllocaInst *AI = new AllocaInst(pDeclType, nullptr, regName, pAnchor);
        AI->setAlignment(kRegCompAlignment);
        reg.pDecl16 = AI;
      }
      if (Decl32Kind == DeclKind::Global) {
        SmallVector<char, 16> regName;
        (Twine("dx.v32.r") + Twine(regIdx)).toStringRef(regName);
        Type *pDeclType = numF32 >= numI32 ? Type::getFloatTy(*m_pCtx)
                                           : Type::getInt32Ty(*m_pCtx);
        GlobalVariable *GV = m_pModule->getGlobalVariable(
            StringRef(regName.data(), regName.size()), true);
        if (!GV) {
          GV = new GlobalVariable(
              *m_pModule, pDeclType, false, GlobalValue::InternalLinkage,
              UndefValue::get(pDeclType), regName, nullptr,
              GlobalVariable::NotThreadLocal, DXIL::kDefaultAddrSpace);
        }
        GV->setAlignment(kRegCompAlignment);
        reg.pDecl32 = GV;
      }
      if (Decl16Kind == DeclKind::Global) {
        SmallVector<char, 16> regName;
        (Twine("dx.v16.r") + Twine(regIdx)).toStringRef(regName);
        Type *pDeclType = numF16 >= numI16 ? Type::getHalfTy(*m_pCtx)
                                           : Type::getInt16Ty(*m_pCtx);
        GlobalVariable *GV = m_pModule->getGlobalVariable(
            StringRef(regName.data(), regName.size()), true);
        if (!GV) {
          GV = new GlobalVariable(
              *m_pModule, pDeclType, false, GlobalValue::InternalLinkage,
              UndefValue::get(pDeclType), regName, nullptr,
              GlobalVariable::NotThreadLocal, DXIL::kDefaultAddrSpace);
        }
        GV->setAlignment(kRegCompAlignment);
        reg.pDecl16 = GV;
      }
    }
  }

  // 3. Replace TempRegLoad/Store with load/store to declared variables.
  for (auto itFn = m_pModule->begin(), endFn = m_pModule->end(); itFn != endFn;
       ++itFn) {
    Function *F = itFn;
    if (F->empty())
      continue;
    DXASSERT_NOMSG(FuncMap.find(F) != FuncMap.end());
    FuncRec &FR = FuncMap[F];

    for (auto itBB = F->begin(), endBB = F->end(); itBB != endBB; ++itBB) {
      BasicBlock *BB = itBB;

      for (auto itInst = BB->begin(), endInst = BB->end(); itInst != endInst;) {
        Instruction *CI = itInst;

        if (DxilInst_TempRegLoad TRL = DxilInst_TempRegLoad(CI)) {
          // Replace TempRegLoad intrinsic with a load.
          unsigned regIdx =
              dyn_cast<ConstantInt>(TRL.get_index())->getZExtValue();
          RegRec &reg = FR.RegMap[regIdx];

          Type *pValType = CI->getType();
          Value *pDecl = (pValType == Type::getFloatTy(*m_pCtx) ||
                          pValType == Type::getInt32Ty(*m_pCtx))
                             ? reg.pDecl32
                             : reg.pDecl16;
          DXASSERT_NOMSG(pValType != nullptr);

          LoadInst *LI = new LoadInst(pDecl, nullptr, CI);
          Value *pBitcastLI = CastValue(LI, pValType, CI);

          CI->replaceAllUsesWith(pBitcastLI);
          ++itInst;
          CI->eraseFromParent();
        } else if (DxilInst_TempRegStore TRS = DxilInst_TempRegStore(CI)) {
          // Replace TempRegStore with a store.
          unsigned regIdx =
              dyn_cast<ConstantInt>(TRS.get_index())->getZExtValue();
          RegRec &reg = FR.RegMap[regIdx];

          Value *pValue = TRS.get_value();
          Type *pValType = pValue->getType();
          Value *pDecl = (pValType == Type::getFloatTy(*m_pCtx) ||
                          pValType == Type::getInt32Ty(*m_pCtx))
                             ? reg.pDecl32
                             : reg.pDecl16;
          DXASSERT_NOMSG(pValType != nullptr);
          Type *pDeclType =
              cast<PointerType>(pDecl->getType())->getElementType();
          Value *pBitcastValueToStore = CastValue(pValue, pDeclType, CI);

          StoreInst *SI = new StoreInst(pBitcastValueToStore, pDecl, CI);
          CI->replaceAllUsesWith(SI);
          ++itInst;
          CI->eraseFromParent();
        } else {
          ++itInst;
        }
      }
    }
  }
}

void DxilCleanup::ConstructSSA() {
  // Construct SSA for r-register live ranges.
#if DXILCLEANUP_DBG
  DXASSERT_NOMSG(!verifyModule(*m_pModule));
#endif

  PassManager PM;
  PM.add(createPromoteMemoryToRegisterPass());
  PM.run(*m_pModule);
}

// Note: this two-pass initialization scheme limits the algorithm to handling
// 2^31 live ranges, instead of 2^32.
#define LIVE_RANGE_UNINITIALIZED (((unsigned)1 << 31))

void DxilCleanup::CollectLiveRanges() {
  // 0. Count and allocate live ranges.
  unsigned LiveRangeCount = 0;
  for (auto itFn = m_pModule->begin(), endFn = m_pModule->end(); itFn != endFn;
       ++itFn) {
    Function *F = itFn;

    for (auto itBB = F->begin(), endBB = F->end(); itBB != endBB; ++itBB) {
      BasicBlock *BB = &*itBB;

      for (auto itInst = BB->begin(), endInst = BB->end(); itInst != endInst;
           ++itInst) {
        Instruction *I = &*itInst;
        Type *pType = I->getType();

        if (!pType->isFloatingPointTy() && !pType->isIntegerTy())
          continue;

        if (m_LiveRangeMap.find(I) != m_LiveRangeMap.end())
          continue;

        // Count live range.
        if (LiveRangeCount & LIVE_RANGE_UNINITIALIZED) {
          // Too many live ranges for our two-pass initialization scheme.
          DXASSERT(false, "otherwise, more than 2^31 live ranges!");
          return;
        }
        CountLiveRangeRec(LiveRangeCount, I);
        LiveRangeCount++;
      }
    }
  }
  m_LiveRanges.resize(LiveRangeCount);

  // 1. Recover live ranges.
  unsigned LRId = 0;
  for (auto itFn = m_pModule->begin(), endFn = m_pModule->end(); itFn != endFn;
       ++itFn) {
    Function *F = itFn;

    for (auto itBB = F->begin(), endBB = F->end(); itBB != endBB; ++itBB) {
      BasicBlock *BB = &*itBB;

      for (auto itInst = BB->begin(), endInst = BB->end(); itInst != endInst;
           ++itInst) {
        Instruction *I = &*itInst;
        Type *pType = I->getType();

        if (!pType->isFloatingPointTy() && !pType->isIntegerTy())
          continue;

        auto it = m_LiveRangeMap.find(I);
        DXASSERT(it != m_LiveRangeMap.end(),
                 "otherwise, instruction not added to m_LiveRangeMap during "
                 "counting stage");
        if (!(it->second & LIVE_RANGE_UNINITIALIZED)) {
          continue;
        }

        // Recover a live range.
        LiveRange &LR = m_LiveRanges[LRId];
        LR.id = LRId++;
        RecoverLiveRangeRec(LR, I);
      }
    }
  }

  // 2. Add bitcast edges.
  for (LiveRange &LR : m_LiveRanges) {
    for (Value *def : LR.defs) {
      for (User *U : def->users()) {
        if (IsDxilBitcast(U)) {
          DXASSERT_NOMSG(m_LiveRangeMap.find(U) != m_LiveRangeMap.end());
          DXASSERT(!(m_LiveRangeMap.find(U)->second & LIVE_RANGE_UNINITIALIZED),
                   "otherwise, live range not initialized!");
          unsigned userLRId = m_LiveRangeMap[U];
          LR.bitcastMap[userLRId]++;
        }
      }
    }
  }

#if DXILCLEANUP_DBG
  // Print live ranges.
  size_t NumDefs = 0;
  dbgs() << "Live ranges:\n";
  for (LiveRange &LR : m_LiveRanges) {
    NumDefs += LR.defs.size();
    dbgs() << "id=" << LR.id << ", F=" << LR.numF << ", I=" << LR.numI
           << ", U=" << LR.numU << ", defs = {";
    for (Value *D : LR.defs) {
      dbgs() << "\n";
      D->dump();
    }
    dbgs() << "}, edges = { ";
    bool bFirst = true;
    for (auto it : LR.bitcastMap) {
      if (!bFirst) {
        dbgs() << ", ";
      }
      dbgs() << "<" << it.first << "," << it.second << ">";
      bFirst = true;
    }
    dbgs() << "}\n";
  }
  DXASSERT_NOMSG(NumDefs == m_LiveRangeMap.size());
#endif
}

void DxilCleanup::CountLiveRangeRec(unsigned LRId, Instruction *pInst) {
  if (m_LiveRangeMap.find(pInst) != m_LiveRangeMap.end()) {
    DXASSERT_NOMSG(m_LiveRangeMap[pInst] == (LRId | LIVE_RANGE_UNINITIALIZED));
    return;
  }

  m_LiveRangeMap[pInst] = LRId | LIVE_RANGE_UNINITIALIZED;

  for (User *U : pInst->users()) {
    if (PHINode *phi = dyn_cast<PHINode>(U)) {
      CountLiveRangeRec(LRId, phi);
    }
  }

  if (PHINode *phi = dyn_cast<PHINode>(pInst)) {
    for (Use &U : phi->operands()) {
      if (Instruction *I = dyn_cast<Instruction>(U.get())) {
        CountLiveRangeRec(LRId, I);
      }
    }
  }
}

void DxilCleanup::RecoverLiveRangeRec(LiveRange &LR, Instruction *pInst) {
  auto it = m_LiveRangeMap.find(pInst);
  DXASSERT_NOMSG(it != m_LiveRangeMap.end());
  if (!(it->second & LIVE_RANGE_UNINITIALIZED)) {
    return;
  }

  it->second &= ~LIVE_RANGE_UNINITIALIZED;
  LR.defs.push_back(pInst);

  for (User *U : pInst->users()) {
    if (PHINode *phi = dyn_cast<PHINode>(U)) {
      RecoverLiveRangeRec(LR, phi);
    } else if (IsDxilBitcast(U)) {
      LR.numU++;
    } else {
      Type *pType = pInst->getType();
      if (pType->isFloatingPointTy()) {
        LR.numF++;
      } else if (pType->isIntegerTy()) {
        LR.numI++;
      } else {
        DXASSERT_NOMSG(false);
      }
    }
  }

  if (PHINode *phi = dyn_cast<PHINode>(pInst)) {
    for (Use &U : phi->operands()) {
      Instruction *I = dyn_cast<Instruction>(U.get());
      if (I) {
        RecoverLiveRangeRec(LR, I);
      } else {
        DXASSERT_NOMSG(dyn_cast<Constant>(U.get()));
      }
    }
  }
}

unsigned DxilCleanup::LiveRange::GetCaseNumber() const {
  if (numI > (numF + numU) || numF > (numI + numU))
    return 1; // Type is known.

  if (numI == (numF + numU) || numF == (numI + numU))
    return 2; // Type may change, but unlikely.

  return 3; // Type is unknown yet. Postpone the decision until more live ranges
            // have types.
}

void DxilCleanup::LiveRange::GuessType(LLVMContext &Ctx) {
  DXASSERT_NOMSG(pNewType == nullptr);
  bool bFlt = false;
  bool bInt = false;
  if (numU == 0) {
    bFlt = numF > numI;
    bInt = numI > numF;
  } else {
    if (numF >= numI + numU) {
      bFlt = true;
    } else if (numI >= numF + numU) {
      bInt = true;
    } else if (numF > numI) {
      bFlt = true;
    } else if (numI > numF) {
      bInt = true;
    }
  }

  Type *pDefType = (*defs.begin())->getType();
  if (!bFlt && !bInt) {
    bFlt = pDefType->isFloatingPointTy();
    bInt = pDefType->isIntegerTy();
  }

  if ((bFlt && pDefType->isFloatingPointTy()) ||
      (bInt && pDefType->isIntegerTy())) {
    pNewType = pDefType;
    return;
  }

  if (bFlt) {
    if (pDefType == Type::getInt16Ty(Ctx)) {
      pNewType = Type::getHalfTy(Ctx);
    } else if (pDefType == Type::getInt32Ty(Ctx)) {
      pNewType = Type::getFloatTy(Ctx);
    } else if (pDefType == Type::getInt64Ty(Ctx)) {
      pNewType = Type::getDoubleTy(Ctx);
    } else {
      DXASSERT_NOMSG(false);
    }
  } else if (bInt) {
    if (pDefType == Type::getHalfTy(Ctx)) {
      pNewType = Type::getInt16Ty(Ctx);
    } else if (pDefType == Type::getFloatTy(Ctx)) {
      pNewType = Type::getInt32Ty(Ctx);
    } else if (pDefType == Type::getDoubleTy(Ctx)) {
      pNewType = Type::getInt64Ty(Ctx);
    } else {
      DXASSERT_NOMSG(false);
    }
  } else {
    DXASSERT_NOMSG(false);
  }
}

bool DxilCleanup::LiveRange::operator<(const LiveRange &o) const {
  unsigned case1 = GetCaseNumber();
  unsigned case2 = o.GetCaseNumber();

  if (case1 != case2)
    return case1 < case2;

  switch (case1) {
  case 1:
  case 2: {
    unsigned n1 = std::max(numI, numF);
    unsigned n2 = std::max(o.numI, o.numF);
    if (n1 != n2)
      return n2 < n1;
    break;
  }
  case 3: {
    double r1 = (double)(numI + numF) / (double)numU;
    double r2 = (double)(o.numI + o.numF) / (double)o.numU;
    if (r1 != r2)
      return r2 < r1;
    if (numU != o.numU)
      return numU < o.numU;
    break;
  }
  default:
    DXASSERT_NOMSG(false);
    break;
  }

  return id < o.id;
}

struct LiveRangeLT {
  LiveRangeLT(const vector<DxilCleanup::LiveRange> &LiveRanges)
      : m_LiveRanges(LiveRanges) {}
  bool operator()(const unsigned i1, const unsigned i2) const {
    const DxilCleanup::LiveRange &lr1 = m_LiveRanges[i1];
    const DxilCleanup::LiveRange &lr2 = m_LiveRanges[i2];
    return lr1 < lr2;
  }

private:
  const vector<DxilCleanup::LiveRange> &m_LiveRanges;
};

void DxilCleanup::InferLiveRangeTypes() {
  set<unsigned, LiveRangeLT> LiveRangeSet{LiveRangeLT(m_LiveRanges)};
  // TODO: Evaluate as candidate for optimization.

  // Initialize queue.
  for (LiveRange &LR : m_LiveRanges) {
    LiveRangeSet.insert(LR.id);
  }

  while (!LiveRangeSet.empty()) {
    unsigned LRId = *LiveRangeSet.cbegin();
    LiveRange &LR = m_LiveRanges[LRId];
    LiveRangeSet.erase(LRId);

    // Assign type.
    LR.GuessType(*m_pCtx);

    // Propagate type assignment to neigboring live ranges.
    for (auto itp : LR.bitcastMap) {
      if (LiveRangeSet.find(itp.first) == LiveRangeSet.end())
        continue;

      unsigned neighborId = itp.first;
      unsigned numLinks = itp.second;
      LiveRangeSet.erase(neighborId);

      LiveRange &neighbor = m_LiveRanges[neighborId];
      if (LR.pNewType->isFloatingPointTy()) {
        neighbor.numF += numLinks;
      } else {
        neighbor.numI += numLinks;
      }
      LiveRangeSet.insert(neighborId);
    }
  }
}

void DxilCleanup::ChangeLiveRangeTypes() {
  for (LiveRange &LR : m_LiveRanges) {
    Type *pType = (*LR.defs.begin())->getType();
    if (pType == LR.pNewType)
      continue;

    // Change live range type.
    SmallDenseMap<Value *, Value *, 4> DefMap;
    // a. Create new defs.
    for (Value *D : LR.defs) {
      Instruction *pInst = dyn_cast<Instruction>(D);
      if (PHINode *phi = dyn_cast<PHINode>(pInst)) {
        PHINode *pNewPhi =
            PHINode::Create(LR.pNewType, phi->getNumIncomingValues(),
                            phi->getName(), phi->getNextNode());
        DefMap[D] = pNewPhi;
      } else {
        DefMap[D] = CastValue(pInst, LR.pNewType, pInst);
      }
    }
    // b. Fix phi uses.
    for (Value *D : LR.defs) {
      if (PHINode *phi = dyn_cast<PHINode>(D)) {
        DXASSERT_NOMSG(DefMap.find(phi) != DefMap.end());
        PHINode *pNewPhi = dyn_cast<PHINode>(DefMap[phi]);

        for (unsigned i = 0; i < phi->getNumIncomingValues(); i++) {
          Value *pVal = phi->getIncomingValue(i);
          BasicBlock *BB = phi->getIncomingBlock(i);
          Value *pNewVal = nullptr;
          if (!isa<Constant>(pVal)) {
            DXASSERT_NOMSG(DefMap.find(pVal) != DefMap.end());
            pNewVal = DefMap[pVal];
          } else {
            pNewVal = CastValue(pVal, pNewPhi->getType(), BB->getTerminator());
          }
          pNewPhi->addIncoming(pNewVal, BB);
        }
      }
    }
    // c. Fix other uses.
    for (Value *D : LR.defs) {
      for (User *U : D->users()) {
        if (isa<PHINode>(U) || IsDxilBitcast(U))
          continue;

        Instruction *pNewInst = dyn_cast<Instruction>(DefMap[D]);
        Value *pRevBitcast = CastValue(pNewInst, pType, pNewInst);
        U->replaceUsesOfWith(D, pRevBitcast);

        // If the new def is a phi we need to be careful about where we place
        // the bitcast. For phis we need to place the bitcast after all the phi
        // defs for the block.
        if (isa<PHINode>(pNewInst) && isa<Instruction>(pRevBitcast) &&
            pRevBitcast != pNewInst) {
          PHINode *pPhi = cast<PHINode>(pNewInst);
          Instruction *pInst = cast<Instruction>(pRevBitcast);
          pInst->removeFromParent();
          pInst->insertBefore(pPhi->getParent()->getFirstInsertionPt());
        }
      }
    }
  }
}

template <typename DxilBitcast1, typename DxilBitcast2>
static bool CleanupBitcastPattern(Instruction *I1) {
  if (DxilBitcast1 BC1 = DxilBitcast1(I1)) {
    Instruction *I2 = dyn_cast<Instruction>(BC1.get_value());
    if (I2) {
      if (DxilBitcast2 BC2 = DxilBitcast2(I2)) {
        I1->replaceAllUsesWith(BC2.get_value());
      }
    }
    return true;
  }
  return false;
}

void DxilCleanup::CleanupPatterns() {
  for (auto itFn = m_pModule->begin(), endFn = m_pModule->end(); itFn != endFn;
       ++itFn) {
    Function *F = itFn;

    for (auto itBB = F->begin(), endBB = F->end(); itBB != endBB; ++itBB) {
      BasicBlock *BB = &*itBB;

      for (auto itInst = BB->begin(), endInst = BB->end(); itInst != endInst;
           ++itInst) {
        Instruction *I1 = &*itInst;

        // Cleanup i1 pattern:
        // %1 = icmp eq i32 %0, 1
        // %2 = sext i1 %1 to i32
        // %3 = icmp ne i32 %2, 0
        // br i1 %3, ...
        //
        // becomes
        // ...
        // br i1 %1, ...
        //
        if (ICmpInst *pICmp = dyn_cast<ICmpInst>(I1)) {
          if (pICmp->getPredicate() != CmpInst::Predicate::ICMP_NE)
            continue;

          Value *O1 = pICmp->getOperand(0);
          if (O1->getType() != Type::getInt32Ty(*m_pCtx))
            continue;
          Value *O2 = pICmp->getOperand(1);
          if (dyn_cast<ConstantInt>(O1))
            std::swap(O1, O2);

          ConstantInt *C = dyn_cast<ConstantInt>(O2);
          if (!C || C->getZExtValue() != 0)
            continue;

          SExtInst *SE = dyn_cast<SExtInst>(O1);
          DXASSERT_NOMSG(!SE || SE->getType() == Type::getInt32Ty(*m_pCtx));
          if (!SE || SE->getSrcTy() != Type::getInt1Ty(*m_pCtx))
            continue;

          I1->replaceAllUsesWith(SE->getOperand(0));

          continue;
        }

        // Cleanup chains of bitcasts:
        // %1 = call float @dx.op.bitcastI32toF32(i32 126, i32 %0)
        // %2 = call i32 @dx.op.bitcastF32toI32(i32 127, float %1)
        // %3 = iadd i32 %2, ...
        //
        // becomes
        // ...
        // %3 = iadd i32 %0, ...
        //
        if (CleanupBitcastPattern<DxilInst_BitcastI32toF32,
                                  DxilInst_BitcastF32toI32>(I1))
          continue;
        if (CleanupBitcastPattern<DxilInst_BitcastF32toI32,
                                  DxilInst_BitcastI32toF32>(I1))
          continue;
        if (CleanupBitcastPattern<DxilInst_BitcastI16toF16,
                                  DxilInst_BitcastF16toI16>(I1))
          continue;
        if (CleanupBitcastPattern<DxilInst_BitcastF16toI16,
                                  DxilInst_BitcastI16toF16>(I1))
          continue;
        if (CleanupBitcastPattern<DxilInst_BitcastI64toF64,
                                  DxilInst_BitcastF64toI64>(I1))
          continue;
        if (CleanupBitcastPattern<DxilInst_BitcastF64toI64,
                                  DxilInst_BitcastI64toF64>(I1))
          continue;

        // Cleanup chains of doubles:
        // %7 = call %dx.types.splitdouble @dx.op.splitDouble.f64(i32 102,
        // double %6) %8 = extractvalue %dx.types.splitdouble %7, 0 %9 =
        // extractvalue %dx.types.splitdouble %7, 1
        // ...
        // %15 = call double @dx.op.makeDouble.f64(i32 101, i32 %8, i32 %9)
        // %16 = call double @dx.op.binary.f64(i32 36, double %15, double
        // 0x3FFC51EB80000000)
        //
        // becomes (%15 -> %6)
        // ...
        // %16 = call double @dx.op.binary.f64(i32 36, double %6, double
        // 0x3FFC51EB80000000)
        //
        if (DxilInst_MakeDouble MD = DxilInst_MakeDouble(I1)) {
          ExtractValueInst *V1 = dyn_cast<ExtractValueInst>(MD.get_hi());
          ExtractValueInst *V2 = dyn_cast<ExtractValueInst>(MD.get_lo());
          if (V1 && V2 &&
              V1->getAggregateOperand() == V2->getAggregateOperand() &&
              V1->getNumIndices() == 1 && V2->getNumIndices() == 1 &&
              *V1->idx_begin() == 1 && *V2->idx_begin() == 0) {
            Instruction *pSDInst =
                dyn_cast<Instruction>(V1->getAggregateOperand());
            if (!pSDInst)
              continue;

            if (DxilInst_SplitDouble SD = DxilInst_SplitDouble(pSDInst)) {
              I1->replaceAllUsesWith(SD.get_value());
            }
          }
          continue;
        }
      }
    }
  }
}

void DxilCleanup::RemoveDeadCode() {
#if DXILCLEANUP_DBG
  DXASSERT_NOMSG(!verifyModule(*m_pModule));
#endif

  PassManager PM;
  PM.add(createDeadCodeEliminationPass());
  PM.run(*m_pModule);
}

Value *DxilCleanup::CastValue(Value *pValue, Type *pToType,
                              Instruction *pOrigInst) {
  Type *pType = pValue->getType();

  if (pType == pToType)
    return pValue;

  const unsigned kNumTypeArgs = 3;
  Type *ArgTypes[kNumTypeArgs];
  DXIL::OpCode OpCode = DXIL::OpCode::NumOpCodes;
  if (pType == Type::getFloatTy(*m_pCtx)) {
    IFTBOOL(pToType == Type::getInt32Ty(*m_pCtx), DXC_E_OPTIMIZATION_FAILED);
    OpCode = DXIL::OpCode::BitcastF32toI32;
    ArgTypes[0] = Type::getInt32Ty(*m_pCtx);
    ArgTypes[1] = Type::getInt32Ty(*m_pCtx);
    ArgTypes[2] = Type::getFloatTy(*m_pCtx);
  } else if (pType == Type::getInt32Ty(*m_pCtx)) {
    IFTBOOL(pToType == Type::getFloatTy(*m_pCtx), DXC_E_OPTIMIZATION_FAILED);
    OpCode = DXIL::OpCode::BitcastI32toF32;
    ArgTypes[0] = Type::getFloatTy(*m_pCtx);
    ArgTypes[1] = Type::getInt32Ty(*m_pCtx);
    ArgTypes[2] = Type::getInt32Ty(*m_pCtx);
  } else if (pType == Type::getHalfTy(*m_pCtx)) {
    IFTBOOL(pToType == Type::getInt16Ty(*m_pCtx), DXC_E_OPTIMIZATION_FAILED);
    OpCode = DXIL::OpCode::BitcastF16toI16;
    ArgTypes[0] = Type::getInt16Ty(*m_pCtx);
    ArgTypes[1] = Type::getInt32Ty(*m_pCtx);
    ArgTypes[2] = Type::getHalfTy(*m_pCtx);
  } else if (pType == Type::getInt16Ty(*m_pCtx)) {
    IFTBOOL(pToType == Type::getHalfTy(*m_pCtx), DXC_E_OPTIMIZATION_FAILED);
    OpCode = DXIL::OpCode::BitcastI16toF16;
    ArgTypes[0] = Type::getHalfTy(*m_pCtx);
    ArgTypes[1] = Type::getInt32Ty(*m_pCtx);
    ArgTypes[2] = Type::getInt16Ty(*m_pCtx);
  } else if (pType == Type::getDoubleTy(*m_pCtx)) {
    IFTBOOL(pToType == Type::getInt64Ty(*m_pCtx), DXC_E_OPTIMIZATION_FAILED);
    OpCode = DXIL::OpCode::BitcastF64toI64;
    ArgTypes[0] = Type::getInt64Ty(*m_pCtx);
    ArgTypes[1] = Type::getInt32Ty(*m_pCtx);
    ArgTypes[2] = Type::getDoubleTy(*m_pCtx);
  } else if (pType == Type::getInt64Ty(*m_pCtx)) {
    IFTBOOL(pToType == Type::getDoubleTy(*m_pCtx), DXC_E_OPTIMIZATION_FAILED);
    OpCode = DXIL::OpCode::BitcastI64toF64;
    ArgTypes[0] = Type::getDoubleTy(*m_pCtx);
    ArgTypes[1] = Type::getInt32Ty(*m_pCtx);
    ArgTypes[2] = Type::getInt64Ty(*m_pCtx);
  } else {
    IFT(DXC_E_OPTIMIZATION_FAILED);
  }

  // Get function.
  std::string funcName =
      (Twine("dx.op.") + Twine(OP::GetOpCodeClassName(OpCode))).str();
  // Try to find exist function with the same name in the module.
  Function *F = m_pModule->getFunction(funcName);
  if (!F) {
    FunctionType *pFT;
    pFT = FunctionType::get(
        ArgTypes[0], ArrayRef<Type *>(&ArgTypes[1], kNumTypeArgs - 1), false);
    F = Function::Create(pFT, GlobalValue::LinkageTypes::ExternalLinkage,
                         funcName, m_pModule);
    F->setCallingConv(CallingConv::C);
    F->addFnAttr(Attribute::NoUnwind);
    F->addFnAttr(Attribute::ReadNone);
  }

  // Create bitcast call.
  const unsigned kNumArgs = 2;
  Value *Args[kNumArgs];
  Args[0] = Constant::getIntegerValue(IntegerType::get(*m_pCtx, 32),
                                      APInt(32, (int)OpCode));
  Args[1] = pValue;
  CallInst *pBitcast = nullptr;
  if (Instruction *pInsertAfter = dyn_cast<Instruction>(pValue)) {
    pBitcast = CallInst::Create(F, ArrayRef<Value *>(&Args[0], kNumArgs), "",
                                pInsertAfter->getNextNode());
  } else {
    pBitcast = CallInst::Create(F, ArrayRef<Value *>(&Args[0], kNumArgs), "",
                                pOrigInst);
  }

  return pBitcast;
}

bool DxilCleanup::IsDxilBitcast(Value *pValue) {
  if (Instruction *pInst = dyn_cast<Instruction>(pValue)) {
    if (OP::IsDxilOpFuncCallInst(pInst)) {
      OP::OpCode opcode = OP::GetDxilOpFuncCallInst(pInst);
      switch (opcode) {
      case OP::OpCode::BitcastF16toI16:
      case OP::OpCode::BitcastF32toI32:
      case OP::OpCode::BitcastF64toI64:
      case OP::OpCode::BitcastI16toF16:
      case OP::OpCode::BitcastI32toF32:
      case OP::OpCode::BitcastI64toF64:
        return true;
      default:
        return false;
      }
    }
  }
  return false;
}

} // namespace DxilCleanupNS

using namespace DxilCleanupNS;

// Publicly exposed interface to pass...
char &llvm::DxilCleanupID = DxilCleanup::ID;

INITIALIZE_PASS_BEGIN(DxilCleanup, "dxil-cleanup",
                      "Optimize DXIL after conversion from DXBC", true, false)
INITIALIZE_PASS_END(DxilCleanup, "dxil-cleanup",
                    "Optimize DXIL after conversion from DXBC", true, false)

namespace llvm {

ModulePass *createDxilCleanupPass() { return new DxilCleanup(); }

} // namespace llvm
