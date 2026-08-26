///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxcPixLiveVariables.cpp                                                   //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Defines the mapping between instructions and the set of live variables    //
// for it.                                                                   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include "dxc/Support/WinIncludes.h"

#include "DxcPixDxilDebugInfo.h"
#include "DxcPixLiveVariables.h"
#include "DxcPixLiveVariables_FragmentIterator.h"
#include "DxilDiaSession.h"

#include "dxc/DXIL/DxilUtil.h"
#include "dxc/Support/Global.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"

#include <unordered_map>

// If a function is inlined, then the scope of variables within that function
// will be that scope. Since many such callers may inline the function, we
// actually need a "scope" that's unique to each "instance" of that function.
// The caller's scope would be unique, but would have the undesirable
// side-effect of smushing all of the function's variables together with the
// caller's variables, at least from the point of view of PIX. The pair of
// scopes (the function's and the caller's) is both unique to that particular
// inlining, and distinct from the caller's scope by itself.
class UniqueScopeForInlinedFunctions {
  llvm::DIScope *FunctionScope;
  llvm::DIScope *InlinedAtScope;

public:
  bool Valid() const { return FunctionScope != nullptr; }
  void AscendScopeHierarchy() {
    // DINamespace has a getScope member (that hides DIScope's)
    // that returns a DIScope directly, but if that namespace
    // is at file-level scope, it will return nullptr.
    if (auto Namespace = llvm::dyn_cast<llvm::DINamespace>(FunctionScope)) {
      if (auto *ContainingScope = Namespace->getScope()) {
        if (FunctionScope == InlinedAtScope)
          InlinedAtScope = FunctionScope = ContainingScope;
        else
          FunctionScope = ContainingScope;
        return;
      }
    }
    const llvm::DITypeIdentifierMap EmptyMap;
    if (FunctionScope == InlinedAtScope)
      InlinedAtScope = FunctionScope =
          FunctionScope->getScope().resolve(EmptyMap);
    else
      FunctionScope = FunctionScope->getScope().resolve(EmptyMap);
  }

  static UniqueScopeForInlinedFunctions Create(llvm::DebugLoc const &DbgLoc,
                                               llvm::DIScope *FunctionScope) {
    UniqueScopeForInlinedFunctions ret;
    ret.FunctionScope = FunctionScope;
    ret.InlinedAtScope =
        llvm::dyn_cast_or_null<llvm::DIScope>(DbgLoc.getInlinedAtScope());
    return ret;
  }

  bool operator==(const UniqueScopeForInlinedFunctions &o) const {
    return FunctionScope == o.FunctionScope &&
           InlinedAtScope == o.InlinedAtScope;
  }

  std::size_t operator()(const UniqueScopeForInlinedFunctions &k) const {
    using std::hash;
    using std::size_t;
    const uint64_t f = reinterpret_cast<uint64_t>(k.FunctionScope);
    const uint64_t s = reinterpret_cast<uint64_t>(k.InlinedAtScope);
    std::hash<uint64_t> h;
    return (h(f) ^ h(s));
  }
};

// ValidateDbgDeclare ensures that all of the bits in
// [FragmentSizeInBits, FragmentOffsetInBits) are currently
// not assigned to a dxil alloca register -- i.e., it
// tries to find overlapping alloca registers -- which should
// never happen -- to report the issue.
void ValidateDbgDeclare(dxil_debug_info::VariableInfo *VarInfo,
                        unsigned FragmentSizeInBits,
                        unsigned FragmentOffsetInBits) {
  // #DSLTodo: When operating on libraries, each exported
  // function is instrumented separately, resulting in
  // overlapping variables. This is not a problem for, e.g.
  // raytracing debugging because WinPIX only ever (so far)
  // invokes debugging on one export at a time.
  // With the advent of DSL, this will have to change...
#if 0  // ndef NDEBUG
  for (unsigned i = 0; i < FragmentSizeInBits; ++i)
  {
    const unsigned BitNum = FragmentOffsetInBits + i;

    VarInfo->m_DbgDeclareValidation.resize(
        std::max<unsigned>(VarInfo->m_DbgDeclareValidation.size(),
                           BitNum + 1));
    assert(!VarInfo->m_DbgDeclareValidation[BitNum]);
    VarInfo->m_DbgDeclareValidation[BitNum] = true;
  }
#endif // !NDEBUG
}

struct dxil_debug_info::LiveVariables::Impl {
  using VariableInfoMap =
      std::unordered_map<llvm::DIVariable *, std::unique_ptr<VariableInfo>>;

  using LiveVarsMap =
      std::unordered_map<UniqueScopeForInlinedFunctions, VariableInfoMap,
                         UniqueScopeForInlinedFunctions>;

  IMalloc *m_pMalloc;
  DxcPixDxilDebugInfo *m_pDxilDebugInfo;
  llvm::Module *m_pModule;
  LiveVarsMap m_LiveVarsDbgDeclare;
  VariableInfoMap m_LiveGlobalVarsDbgDeclare;

  void Init(IMalloc *pMalloc, DxcPixDxilDebugInfo *pDxilDebugInfo,
            llvm::Module *pModule);

  void Init_DbgDeclare(llvm::DbgDeclareInst *DbgDeclare);

  VariableInfo *AssignValueToOffset(VariableInfoMap *VarInfoMap,
                                    llvm::DIVariable *Var, llvm::Value *Address,
                                    unsigned FragmentIndex,
                                    unsigned FragmentOffsetInBits);

  bool IsVariableLive(const VariableInfoMap::value_type &VarAndInfo,
                      const llvm::DIScope *S, const llvm::DebugLoc &DL);
};

void dxil_debug_info::LiveVariables::Impl::Init(
    IMalloc *pMalloc, DxcPixDxilDebugInfo *pDxilDebugInfo,
    llvm::Module *pModule) {
  m_pMalloc = pMalloc;
  m_pDxilDebugInfo = pDxilDebugInfo;
  m_pModule = pModule;

  llvm::Function *DbgDeclareFn =
      llvm::Intrinsic::getDeclaration(m_pModule, llvm::Intrinsic::dbg_declare);

  for (llvm::User *U : DbgDeclareFn->users()) {
    if (auto *DbgDeclare = llvm::dyn_cast<llvm::DbgDeclareInst>(U)) {
      Init_DbgDeclare(DbgDeclare);
    }
  }
}

void dxil_debug_info::LiveVariables::Impl::Init_DbgDeclare(
    llvm::DbgDeclareInst *DbgDeclare) {
  llvm::Value *Address = DbgDeclare->getAddress();
  auto *Variable = DbgDeclare->getVariable();
  auto *Expression = DbgDeclare->getExpression();

  if (Address == nullptr || Variable == nullptr || Expression == nullptr) {
    return;
  }

  auto *AddressAsAlloca = llvm::dyn_cast<llvm::AllocaInst>(Address);
  if (AddressAsAlloca == nullptr) {
    return;
  }

  auto S = UniqueScopeForInlinedFunctions::Create(DbgDeclare->getDebugLoc(),
                                                  Variable->getScope());
  if (!S.Valid()) {
    return;
  }

  auto Iter = CreateMemberIterator(DbgDeclare, m_pModule->getDataLayout(),
                                   AddressAsAlloca, Expression);

  if (!Iter) {
    // MemberIterator creation failure, this skip this var.
    return;
  }

  VariableInfoMap *LiveVarInfoMap;
  if (Variable->getName().startswith("global.")) {
    LiveVarInfoMap = &m_LiveGlobalVarsDbgDeclare;
  } else {
    LiveVarInfoMap = &m_LiveVarsDbgDeclare[S];
  }

  unsigned FragmentIndex;
  while (Iter->Next(&FragmentIndex)) {
    const unsigned FragmentSizeInBits = Iter->SizeInBits(FragmentIndex);
    const unsigned FragmentOffsetInBits = Iter->OffsetInBits(FragmentIndex);

    VariableInfo *VarInfo = AssignValueToOffset(
        LiveVarInfoMap, Variable, Address, FragmentIndex, FragmentOffsetInBits);

    // SROA can split structs so that multiple allocas back the same variable.
    // In this case the expression will be empty
    if (Expression->getNumElements() != 0) {
      ValidateDbgDeclare(VarInfo, FragmentSizeInBits, FragmentOffsetInBits);
    }
  }
}

dxil_debug_info::VariableInfo *
dxil_debug_info::LiveVariables::Impl::AssignValueToOffset(
    VariableInfoMap *VarInfoMap, llvm::DIVariable *Variable,
    llvm::Value *Address, unsigned FragmentIndex,
    unsigned FragmentOffsetInBits) {
  // FragmentIndex is the index within the alloca'd value
  // FragmentOffsetInBits is the offset within the HLSL variable
  // that maps to Address[FragmentIndex]
  auto it = VarInfoMap->find(Variable);
  if (it == VarInfoMap->end()) {
    auto InsertIt =
        VarInfoMap->emplace(Variable, std::make_unique<VariableInfo>(Variable));
    assert(InsertIt.second);
    it = InsertIt.first;
  }
  auto *VarInfo = it->second.get();
  auto &FragmentLocation = VarInfo->m_ValueLocationMap[FragmentOffsetInBits];
  FragmentLocation.m_V = Address;
  FragmentLocation.m_FragmentIndex = FragmentIndex;

  return VarInfo;
}

dxil_debug_info::LiveVariables::LiveVariables() = default;

dxil_debug_info::LiveVariables::~LiveVariables() = default;

HRESULT
dxil_debug_info::LiveVariables::Init(DxcPixDxilDebugInfo *pDxilDebugInfo) {
  Clear();
  m_pImpl->Init(pDxilDebugInfo->GetMallocNoRef(), pDxilDebugInfo,
                pDxilDebugInfo->GetModuleRef());
  return S_OK;
}

void dxil_debug_info::LiveVariables::Clear() {
  m_pImpl.reset(new dxil_debug_info::LiveVariables::Impl());
}

HRESULT dxil_debug_info::LiveVariables::GetLiveVariablesAtInstruction(
    llvm::Instruction *IP, IDxcPixDxilLiveVariables **ppResult) const {
  DXASSERT(IP != nullptr, "else IP should not be nullptr");
  DXASSERT(ppResult != nullptr, "else Result should not be nullptr");

  std::vector<const VariableInfo *> LiveVars;
  std::set<std::string> LiveVarsName;

  const llvm::DebugLoc &DL = IP->getDebugLoc();

  if (!DL) {
    return E_FAIL;
  }

  auto S = UniqueScopeForInlinedFunctions::Create(DL, DL->getScope());
  if (!S.Valid()) {
    return E_FAIL;
  }

  while (S.Valid()) {
    auto it = m_pImpl->m_LiveVarsDbgDeclare.find(S);
    if (it != m_pImpl->m_LiveVarsDbgDeclare.end()) {
      for (const auto &VarAndInfo : it->second) {
        auto *Var = VarAndInfo.first;
        llvm::StringRef VarName = Var->getName();
        if (Var->getLine() > DL.getLine()) {
          // Defined later in the HLSL source.
          continue;
        }
        if (VarName.empty()) {
          // No name?...
          continue;
        }
        if (!LiveVarsName.insert(VarAndInfo.first->getName()).second) {
          // There's a variable with the same name; use the
          // previous one instead.
          return false;
        }
        LiveVars.emplace_back(VarAndInfo.second.get());
      }
    }
    S.AscendScopeHierarchy();
  }
  for (const auto &VarAndInfo : m_pImpl->m_LiveGlobalVarsDbgDeclare) {
    // Only consider references to the global variable that are in the same
    // function as the instruction.
    if (hlsl::dxilutil::DemangleFunctionName(
            IP->getParent()->getParent()->getName()) ==
        VarAndInfo.first->getScope()->getName()) {
      if (!LiveVarsName.insert(VarAndInfo.first->getName()).second) {
        // There shouldn't ever be a global variable with the same
        // name, but it doesn't hurt to check
        continue;
      }
      LiveVars.emplace_back(VarAndInfo.second.get());
    }
  }
  return CreateDxilLiveVariables(m_pImpl->m_pDxilDebugInfo, std::move(LiveVars),
                                 ppResult);
}
