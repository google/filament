///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilDbgValueToDbgDeclare.cpp                                              //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// Converts calls to llvm.dbg.value to llvm.dbg.declare + alloca + stores.   //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilModule.h"
#include "dxc/DXIL/DxilOperations.h"
#include "dxc/DXIL/DxilResourceBase.h"
#include "dxc/DxilPIXPasses/DxilPIXPasses.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"

#include "PixPassHelpers.h"
using namespace PIXPassHelpers;

using namespace llvm;

// #define VALUE_TO_DECLARE_LOGGING

#ifdef VALUE_TO_DECLARE_LOGGING
#ifndef PIX_DEBUG_DUMP_HELPER
#error Turn on PIX_DEBUG_DUMP_HELPER in PixPassHelpers.h
#endif
#define VALUE_TO_DECLARE_LOG Log
#else
#define VALUE_TO_DECLARE_LOG(...)
#endif

#define DEBUG_TYPE "dxil-dbg-value-to-dbg-declare"

namespace {
using OffsetInBits = unsigned;
using SizeInBits = unsigned;
struct Offsets {
  OffsetInBits Aligned;
  OffsetInBits Packed;
};

// DITypePeelTypeAlias peels const, typedef, and other alias types off of Ty,
// returning the unalised type.
static llvm::DIType *DITypePeelTypeAlias(llvm::DIType *Ty) {
  if (auto *DerivedTy = llvm::dyn_cast<llvm::DIDerivedType>(Ty)) {
    const llvm::DITypeIdentifierMap EmptyMap;
    switch (DerivedTy->getTag()) {
    case llvm::dwarf::DW_TAG_restrict_type:
    case llvm::dwarf::DW_TAG_reference_type:
    case llvm::dwarf::DW_TAG_const_type:
    case llvm::dwarf::DW_TAG_typedef:
    case llvm::dwarf::DW_TAG_pointer_type:
      return DITypePeelTypeAlias(DerivedTy->getBaseType().resolve(EmptyMap));
    case llvm::dwarf::DW_TAG_member:
      return DITypePeelTypeAlias(DerivedTy->getBaseType().resolve(EmptyMap));
    }
  }

  return Ty;
}

llvm::DIBasicType *BaseTypeIfItIsBasicAndLarger(llvm::DIType *Ty) {
  // Working around problems with bitfield size/alignment:
  // For bitfield types, size may be < 32, but the underlying type
  // will have the size of that basic type, e.g. 32 for ints.
  // By contrast, for min16float, size will be 16, but align will be 16 or 32
  // depending on whether or not 16-bit is enabled.
  // So if we find a disparity in size, we can assume it's not e.g. min16float.
  auto *baseType = DITypePeelTypeAlias(Ty);
  if (Ty->getSizeInBits() != 0 &&
      Ty->getSizeInBits() < baseType->getSizeInBits())
    return llvm::dyn_cast<llvm::DIBasicType>(baseType);
  return nullptr;
}

// OffsetManager is used to map between "packed" and aligned offsets.
//
// For example, the aligned offsets for a struct [float, half, int, double]
// will be {0, 32, 64, 128} (assuming 32 bit alignments for ints, and 64
// bit for doubles), while the packed offsets will be {0, 32, 48, 80}.
//
// This mapping makes it easier to deal with llvm.dbg.values whose value
// operand does not match exactly the Variable operand's type.
class OffsetManager {
  unsigned DescendTypeToGetAlignMask(llvm::DIType *Ty) {
    unsigned AlignMask = Ty->getAlignInBits();
    if (BaseTypeIfItIsBasicAndLarger(Ty))
      AlignMask = 0;
    else {
      auto *DerivedTy = llvm::dyn_cast<llvm::DIDerivedType>(Ty);
      if (DerivedTy != nullptr) {
        // Working around a bug where byte size is stored instead of bit size
        if (AlignMask == 4 && Ty->getSizeInBits() == 32) {
          AlignMask = 32;
        }
        if (AlignMask == 0) {
          const llvm::DITypeIdentifierMap EmptyMap;
          switch (DerivedTy->getTag()) {
          case llvm::dwarf::DW_TAG_restrict_type:
          case llvm::dwarf::DW_TAG_reference_type:
          case llvm::dwarf::DW_TAG_const_type:
          case llvm::dwarf::DW_TAG_typedef: {
            llvm::DIType *baseType = DerivedTy->getBaseType().resolve(EmptyMap);
            if (baseType != nullptr) {
              return DescendTypeToGetAlignMask(baseType);
            }
          }
          }
        }
      }
    }
    return AlignMask;
  }

public:
  OffsetManager() = default;

  // AlignTo aligns the current aligned offset to Ty's natural alignment.
  void AlignTo(llvm::DIType *Ty) {
    unsigned AlignMask = DescendTypeToGetAlignMask(Ty);
    if (AlignMask) {
      VALUE_TO_DECLARE_LOG("Aligning to %d", AlignMask);
      m_CurrentAlignedOffset =
          llvm::RoundUpToAlignment(m_CurrentAlignedOffset, AlignMask);
    } else {
      VALUE_TO_DECLARE_LOG("Failed to find alignment");
    }
  }

  // Add is used to "add" an aggregate element (struct field, array element)
  // at the current aligned/packed offsets, bumping them by Ty's size.
  Offsets Add(llvm::DIBasicType *Ty, unsigned sizeOverride) {
    VALUE_TO_DECLARE_LOG("Adding known type at aligned %d / packed %d, size %d",
                         m_CurrentAlignedOffset, m_CurrentPackedOffset,
                         Ty->getSizeInBits());

    m_PackedOffsetToAlignedOffset[m_CurrentPackedOffset] =
        m_CurrentAlignedOffset;
    m_AlignedOffsetToPackedOffset[m_CurrentAlignedOffset] =
        m_CurrentPackedOffset;

    const Offsets Ret = {m_CurrentAlignedOffset, m_CurrentPackedOffset};
    unsigned size = sizeOverride != 0 ? sizeOverride : Ty->getSizeInBits();
    m_CurrentPackedOffset += size;
    m_CurrentAlignedOffset += size;

    return Ret;
  }

  // AlignToAndAddUnhandledType is used for error handling when Ty
  // could not be handled by the transformation. This is a best-effort
  // way to continue the pass by ignoring the current type and hoping
  // that adding Ty as a blob other fields/elements added will land
  // in the proper offset.
  void AlignToAndAddUnhandledType(llvm::DIType *Ty) {
    VALUE_TO_DECLARE_LOG(
        "Adding unhandled type at aligned %d / packed %d, size %d",
        m_CurrentAlignedOffset, m_CurrentPackedOffset, Ty->getSizeInBits());
    AlignTo(Ty);
    m_CurrentPackedOffset += Ty->getSizeInBits();
    m_CurrentAlignedOffset += Ty->getSizeInBits();
  }

  void AddResourceType(llvm::DIType *Ty) {
    VALUE_TO_DECLARE_LOG(
        "Adding resource type at aligned %d / packed %d, size %d",
        m_CurrentAlignedOffset, m_CurrentPackedOffset, Ty->getSizeInBits());
    m_PackedOffsetToAlignedOffset[m_CurrentPackedOffset] =
        m_CurrentAlignedOffset;
    m_AlignedOffsetToPackedOffset[m_CurrentAlignedOffset] =
        m_CurrentPackedOffset;

    m_CurrentPackedOffset += Ty->getSizeInBits();
    m_CurrentAlignedOffset += Ty->getSizeInBits();
  }

  bool GetAlignedOffsetFromPackedOffset(OffsetInBits PackedOffset,
                                        OffsetInBits *AlignedOffset) const {
    return GetOffsetWithMap(m_PackedOffsetToAlignedOffset, PackedOffset,
                            AlignedOffset);
  }

  bool GetPackedOffsetFromAlignedOffset(OffsetInBits AlignedOffset,
                                        OffsetInBits *PackedOffset) const {
    return GetOffsetWithMap(m_AlignedOffsetToPackedOffset, AlignedOffset,
                            PackedOffset);
  }

  OffsetInBits GetCurrentPackedOffset() const { return m_CurrentPackedOffset; }

  OffsetInBits GetCurrentAlignedOffset() const {
    return m_CurrentAlignedOffset;
  }

private:
  OffsetInBits m_CurrentPackedOffset = 0;
  OffsetInBits m_CurrentAlignedOffset = 0;

  using OffsetMap = std::unordered_map<OffsetInBits, OffsetInBits>;

  OffsetMap m_PackedOffsetToAlignedOffset;
  OffsetMap m_AlignedOffsetToPackedOffset;

  static bool GetOffsetWithMap(const OffsetMap &Map, OffsetInBits SrcOffset,
                               OffsetInBits *DstOffset) {
    auto it = Map.find(SrcOffset);
    if (it == Map.end()) {
      return false;
    }

    *DstOffset = it->second;
    return true;
  }
};

// VariableRegisters contains the logic for traversing a DIType T and
// creating AllocaInsts that map back to a specific offset within T.
class VariableRegisters {
public:
  VariableRegisters(llvm::DebugLoc const &m_dbgLoc,
                    llvm::BasicBlock::iterator allocaInsertionPoint,
                    llvm::DIVariable *Variable, llvm::DIType *Ty,
                    llvm::Module *M);

  llvm::AllocaInst *
  GetRegisterForAlignedOffset(OffsetInBits AlignedOffset) const;

  const OffsetManager &GetOffsetManager() const { return m_Offsets; }

  static SizeInBits GetVariableSizeInbits(DIVariable *Var);

private:
  void PopulateAllocaMap(llvm::DIType *Ty);

  void PopulateAllocaMap_BasicType(llvm::DIBasicType *Ty,
                                   unsigned sizeOverride);

  void PopulateAllocaMap_ArrayType(llvm::DICompositeType *Ty);

  void PopulateAllocaMap_StructType(llvm::DICompositeType *Ty);

  llvm::DILocation *GetVariableLocation() const;
  llvm::Value *GetMetadataAsValue(llvm::Metadata *M) const;
  llvm::DIExpression *GetDIExpression(llvm::DIType *Ty, OffsetInBits Offset,
                                      SizeInBits ParentSize,
                                      unsigned sizeOverride) const;

  llvm::DebugLoc const &m_dbgLoc;
  llvm::DIVariable *m_Variable = nullptr;
  llvm::IRBuilder<> m_B;
  llvm::Function *m_DbgDeclareFn = nullptr;

  OffsetManager m_Offsets;
  std::unordered_map<OffsetInBits, llvm::AllocaInst *> m_AlignedOffsetToAlloca;
};

struct GlobalEmbeddedArrayElementStorage {
  std::string Name;
  OffsetInBits Offset;
  SizeInBits Size;
};
using GlobalVariableToLocalMirrorMap =
    std::map<llvm::Function const *, llvm::DILocalVariable *>;
struct LocalMirrorsAndStorage {
  std::vector<GlobalEmbeddedArrayElementStorage> ArrayElementStorage;
  GlobalVariableToLocalMirrorMap LocalMirrors;
};
using GlobalStorageMap =
    std::map<llvm::DIGlobalVariable *, LocalMirrorsAndStorage>;

class DxilDbgValueToDbgDeclare : public llvm::ModulePass {
public:
  static char ID;
  DxilDbgValueToDbgDeclare() : llvm::ModulePass(ID) {}
  bool runOnModule(llvm::Module &M) override;

private:
  void handleDbgValue(llvm::Module &M, llvm::DbgValueInst *DbgValue);
  bool handleStoreIfDestIsGlobal(llvm::Module &M,
                                 GlobalStorageMap &GlobalStorage,
                                 llvm::StoreInst *Store);

  std::unordered_map<llvm::DIVariable *, std::unique_ptr<VariableRegisters>>
      m_Registers;
};
} // namespace

char DxilDbgValueToDbgDeclare::ID = 0;

struct ValueAndOffset {
  llvm::Value *m_V;
  OffsetInBits m_PackedOffset;
};

// SplitValue splits an llvm::Value into possibly multiple
// scalar Values. Those scalar values will later be "stored"
// into their corresponding register.
static OffsetInBits SplitValue(llvm::Value *V, OffsetInBits CurrentOffset,
                               std::vector<ValueAndOffset> *Values,
                               llvm::IRBuilder<> &B) {
  auto *VTy = V->getType();
  if (auto *ArrTy = llvm::dyn_cast<llvm::ArrayType>(VTy)) {
    for (unsigned i = 0; i < ArrTy->getNumElements(); ++i) {
      CurrentOffset =
          SplitValue(B.CreateExtractValue(V, {i}), CurrentOffset, Values, B);
    }
  } else if (auto *StTy = llvm::dyn_cast<llvm::StructType>(VTy)) {
    for (unsigned i = 0; i < StTy->getNumElements(); ++i) {
      CurrentOffset =
          SplitValue(B.CreateExtractValue(V, {i}), CurrentOffset, Values, B);
    }
  } else if (auto *VecTy = llvm::dyn_cast<llvm::VectorType>(VTy)) {
    for (unsigned i = 0; i < VecTy->getNumElements(); ++i) {
      CurrentOffset =
          SplitValue(B.CreateExtractElement(V, i), CurrentOffset, Values, B);
    }
  } else {
    assert(VTy->isFloatTy() || VTy->isDoubleTy() || VTy->isHalfTy() ||
           VTy->isIntegerTy(32) || VTy->isIntegerTy(64) ||
           VTy->isIntegerTy(16) || VTy->isPointerTy());
    Values->emplace_back(ValueAndOffset{V, CurrentOffset});
    CurrentOffset += VTy->getScalarSizeInBits();
  }

  return CurrentOffset;
}

// A more convenient version of SplitValue.
static std::vector<ValueAndOffset>
SplitValue(llvm::Value *V, OffsetInBits CurrentOffset, llvm::IRBuilder<> &B) {
  std::vector<ValueAndOffset> Ret;
  SplitValue(V, CurrentOffset, &Ret, B);
  return Ret;
}

// Convenient helper for parsing a DIExpression's offset.
static OffsetInBits GetAlignedOffsetFromDIExpression(llvm::DIExpression *Exp) {
  if (!Exp->isBitPiece()) {
    return 0;
  }

  return Exp->getBitPieceOffset();
}

llvm::DISubprogram *GetFunctionDebugInfo(llvm::Module &M, llvm::Function *fn) {
  auto FnMap = makeSubprogramMap(M);
  return FnMap[fn];
}

GlobalVariableToLocalMirrorMap
GenerateGlobalToLocalMirrorMap(llvm::Module &M, llvm::DIGlobalVariable *DIGV) {
  auto &Functions = M.getFunctionList();

  std::string LocalMirrorOfGlobalName =
      std::string("global.") + std::string(DIGV->getName());

  GlobalVariableToLocalMirrorMap ret;
  DenseMap<const Function *, DISubprogram *> FnMap;

  for (llvm::Function const &fn : Functions) {
    auto &blocks = fn.getBasicBlockList();
    if (!blocks.empty()) {
      auto &LocalMirror = ret[&fn];
      for (auto &block : blocks) {
        bool breakOut = false;
        for (auto &instruction : block) {
          if (auto const *DbgValue =
                  llvm::dyn_cast<llvm::DbgValueInst>(&instruction)) {
            auto *Variable = DbgValue->getVariable();
            if (Variable->getName().equals(LocalMirrorOfGlobalName)) {
              LocalMirror = Variable;
              breakOut = true;
              break;
            }
          }
          if (auto const *DbgDeclare =
                  llvm::dyn_cast<llvm::DbgDeclareInst>(&instruction)) {
            auto *Variable = DbgDeclare->getVariable();
            if (Variable->getName().equals(LocalMirrorOfGlobalName)) {
              LocalMirror = Variable;
              breakOut = true;
              break;
            }
          }
        }
        if (breakOut)
          break;
      }
      if (LocalMirror == nullptr) {
        // If we didn't find a dbg.value for any member of this
        // DIGlobalVariable, then no local mirror exists. We must manufacture
        // one.
        if (FnMap.empty()) {
          FnMap = makeSubprogramMap(M);
        }
        auto DIFn = FnMap[&fn];
        if (DIFn != nullptr) {
          const llvm::DITypeIdentifierMap EmptyMap;
          auto DIGVType = DIGV->getType().resolve(EmptyMap);
          DIBuilder DbgInfoBuilder(M);
          LocalMirror = DbgInfoBuilder.createLocalVariable(
              dwarf::DW_TAG_auto_variable, DIFn, LocalMirrorOfGlobalName,
              DIFn->getFile(), DIFn->getLine(), DIGVType);
        }
      }
    }
  }
  return ret;
}

std::vector<GlobalEmbeddedArrayElementStorage>
DescendTypeAndFindEmbeddedArrayElements(llvm::StringRef VariableName,
                                        uint64_t AccumulatedMemberOffset,
                                        llvm::DIType *Ty, uint64_t OffsetToSeek,
                                        uint64_t SizeToSeek) {
  const llvm::DITypeIdentifierMap EmptyMap;
  if (auto *DerivedTy = llvm::dyn_cast<llvm::DIDerivedType>(Ty)) {
    auto BaseTy = DerivedTy->getBaseType().resolve(EmptyMap);
    auto storage = DescendTypeAndFindEmbeddedArrayElements(
        VariableName, AccumulatedMemberOffset, BaseTy, OffsetToSeek,
        SizeToSeek);
    if (!storage.empty()) {
      return storage;
    }
  } else if (auto *CompositeTy = llvm::dyn_cast<llvm::DICompositeType>(Ty)) {
    switch (CompositeTy->getTag()) {
    case llvm::dwarf::DW_TAG_array_type: {
      for (auto Element : CompositeTy->getElements()) {
        // First element for an array is DISubrange
        if (auto Subrange = llvm::dyn_cast<DISubrange>(Element)) {
          auto ElementTy = CompositeTy->getBaseType().resolve(EmptyMap);
          if (auto *BasicTy = llvm::dyn_cast<llvm::DIBasicType>(ElementTy)) {
            bool CorrectLowerOffset = AccumulatedMemberOffset == OffsetToSeek;
            bool CorrectUpperOffset =
                AccumulatedMemberOffset +
                    Subrange->getCount() * BasicTy->getSizeInBits() ==
                OffsetToSeek + SizeToSeek;
            if (BasicTy != nullptr && CorrectLowerOffset &&
                CorrectUpperOffset) {
              std::vector<GlobalEmbeddedArrayElementStorage> storage;
              for (int64_t i = 0; i < Subrange->getCount(); ++i) {
                auto ElementOffset =
                    AccumulatedMemberOffset + i * BasicTy->getSizeInBits();
                GlobalEmbeddedArrayElementStorage element;
                element.Name = VariableName.str() + "." + std::to_string(i);
                element.Offset = static_cast<OffsetInBits>(ElementOffset);
                element.Size =
                    static_cast<SizeInBits>(BasicTy->getSizeInBits());
                storage.push_back(std::move(element));
              }
              return storage;
            }
          }

          // If we didn't succeed and return above, then we need to process each
          // element in the array
          std::vector<GlobalEmbeddedArrayElementStorage> storage;
          for (int64_t i = 0; i < Subrange->getCount(); ++i) {
            auto elementStorage = DescendTypeAndFindEmbeddedArrayElements(
                VariableName,
                AccumulatedMemberOffset + ElementTy->getSizeInBits() * i,
                ElementTy, OffsetToSeek, SizeToSeek);
            std::move(elementStorage.begin(), elementStorage.end(),
                      std::back_inserter(storage));
          }
          if (!storage.empty()) {
            return storage;
          }
        }
      }
      for (auto Element : CompositeTy->getElements()) {
        // First element for an array is DISubrange
        if (auto Subrange = llvm::dyn_cast<DISubrange>(Element)) {
          auto ElementType = CompositeTy->getBaseType().resolve(EmptyMap);
          for (int64_t i = 0; i < Subrange->getCount(); ++i) {
            auto storage = DescendTypeAndFindEmbeddedArrayElements(
                VariableName,
                AccumulatedMemberOffset + ElementType->getSizeInBits() * i,
                ElementType, OffsetToSeek, SizeToSeek);
            if (!storage.empty()) {
              return storage;
            }
          }
        }
      }
    } break;
    case llvm::dwarf::DW_TAG_structure_type:
    case llvm::dwarf::DW_TAG_class_type: {
      for (auto Element : CompositeTy->getElements()) {
        if (auto diMember = llvm::dyn_cast<DIType>(Element)) {
          auto storage = DescendTypeAndFindEmbeddedArrayElements(
              VariableName,
              AccumulatedMemberOffset + diMember->getOffsetInBits(), diMember,
              OffsetToSeek, SizeToSeek);
          if (!storage.empty()) {
            return storage;
          }
        }
      }
    } break;
    }
  }
  return {};
}

GlobalStorageMap GatherGlobalEmbeddedArrayStorage(llvm::Module &M) {
  GlobalStorageMap ret;
  auto DebugFinder = llvm::make_unique<llvm::DebugInfoFinder>();
  DebugFinder->processModule(M);
  auto GlobalVariables = DebugFinder->global_variables();

  // First find the list of global variables that represent HLSL global statics:
  const llvm::DITypeIdentifierMap EmptyMap;
  SmallVector<llvm::DIGlobalVariable *, 8> GlobalStaticVariables;
  for (llvm::DIGlobalVariable *DIGV : GlobalVariables) {
    if (DIGV->isLocalToUnit()) {
      llvm::DIType *DIGVType = DIGV->getType().resolve(EmptyMap);
      // We're only interested in aggregates, since only they might have
      // embedded arrays:
      if (isa<llvm::DICompositeType>(DIGVType)) {
        auto LocalMirrors = GenerateGlobalToLocalMirrorMap(M, DIGV);
        if (!LocalMirrors.empty()) {
          GlobalStaticVariables.push_back(DIGV);
          ret[DIGV].LocalMirrors = std::move(LocalMirrors);
        }
      }
    }
  }

  // Now find any globals that represent embedded arrays inside the global
  // statics
  for (auto HLSLStruct : GlobalStaticVariables) {
    for (llvm::DIGlobalVariable *DIGV : GlobalVariables) {
      if (DIGV != HLSLStruct && !DIGV->isLocalToUnit()) {
        llvm::DIType *DIGVType = DIGV->getType().resolve(EmptyMap);
        if (auto *DIGVDerivedType =
                llvm::dyn_cast<llvm::DIDerivedType>(DIGVType)) {
          if (DIGVDerivedType->getTag() == llvm::dwarf::DW_TAG_member) {
            // This type is embedded within the containing DIGSV type
            const llvm::DITypeIdentifierMap EmptyMap;
            auto *Ty = HLSLStruct->getType().resolve(EmptyMap);
            auto Storage = DescendTypeAndFindEmbeddedArrayElements(
                DIGV->getName(), 0, Ty, DIGVDerivedType->getOffsetInBits(),
                DIGVDerivedType->getSizeInBits());
            auto &ArrayStorage = ret[HLSLStruct].ArrayElementStorage;
            std::move(Storage.begin(), Storage.end(),
                      std::back_inserter(ArrayStorage));
          }
        }
      }
    }
  }
  return ret;
}

bool DxilDbgValueToDbgDeclare::runOnModule(llvm::Module &M) {
  auto GlobalEmbeddedArrayStorage = GatherGlobalEmbeddedArrayStorage(M);

  bool Changed = false;

  auto &Functions = M.getFunctionList();
  for (auto &fn : Functions) {
    llvm::SmallPtrSet<Value *, 16> RayQueryHandles;
    PIXPassHelpers::FindRayQueryHandlesForFunction(&fn, RayQueryHandles);
    // #DSLTodo: We probably need to merge the list of variables for each
    // export into one set so that WinPIX shader debugging can follow a
    // thread through any function within a given module. (Unless PIX
    // chooses to launch a new debugging session whenever control passes
    // from one function to another.) For now, it's sufficient to treat each
    // exported function as having completely separate variables by clearing
    // this member:
    m_Registers.clear();
    // Note: they key problem here is variables in common functions called
    // by multiple exported functions. The DILocalVariables in the common
    // function will be exactly the same objects no matter which export
    // called the common function, so the instrumentation here gets a bit
    // confused that the same variable is present in two functions and ends
    // up pointing one function to allocas in another function. (This is
    // easy to repro: comment out the above clear(), and run
    // PixTest::PixStructAnnotation_Lib_DualRaygen.) Not sure what the right
    // path forward is: might be that we have to tag m_Registers with the
    // exported function, and maybe write out a function identifier during
    // debug instrumentation...
    auto &blocks = fn.getBasicBlockList();
    if (!blocks.empty()) {
      for (auto &block : blocks) {
        std::vector<Instruction *> instructions;
        for (auto &instruction : block) {
          instructions.push_back(&instruction);
        }
        // Handle store instructions before handling dbg.value, since the
        // latter will add store instructions that we don't need to examine.
        // Why do we handle store instructions? It's for the case of
        // non-const global statics that are backed by an llvm global,
        // rather than an alloca. In the llvm global case, there is no
        // debug linkage between the store and the HLSL variable being
        // modified. But we can patch together enough knowledge about those
        // from the lists of such globals (HLSL and llvm) and comparing the
        // lists.
        for (auto &instruction : instructions) {
          if (auto *Store = llvm::dyn_cast<llvm::StoreInst>(instruction)) {
            Changed =
                handleStoreIfDestIsGlobal(M, GlobalEmbeddedArrayStorage, Store);
          }
        }
        for (auto &instruction : instructions) {
          if (auto *DbgValue =
                  llvm::dyn_cast<llvm::DbgValueInst>(instruction)) {
            llvm::Value *V = DbgValue->getValue();
            if (RayQueryHandles.count(V) != 0)
              continue;
            Changed = true;
            handleDbgValue(M, DbgValue);
            DbgValue->eraseFromParent();
          }
        }
      }
    }
  }
  return Changed;
}

static llvm::DIType *FindStructMemberTypeAtOffset(llvm::DICompositeType *Ty,
                                                  uint64_t Offset,
                                                  uint64_t Size);

static llvm::DIType *FindMemberTypeAtOffset(llvm::DIType *Ty, uint64_t Offset,
                                            uint64_t Size) {
  VALUE_TO_DECLARE_LOG("PopulateAllocaMap for type tag %d", Ty->getTag());
  const llvm::DITypeIdentifierMap EmptyMap;
  if (auto *DerivedTy = llvm::dyn_cast<llvm::DIDerivedType>(Ty)) {
    switch (DerivedTy->getTag()) {
    default:
      assert(!"Unhandled DIDerivedType");
      return nullptr;
    case llvm::dwarf::DW_TAG_arg_variable: // "this" pointer
    case llvm::dwarf::DW_TAG_pointer_type: // "this" pointer
                                           // what to do here?
      return nullptr;
    case llvm::dwarf::DW_TAG_restrict_type:
    case llvm::dwarf::DW_TAG_reference_type:
    case llvm::dwarf::DW_TAG_const_type:
    case llvm::dwarf::DW_TAG_typedef:
      return FindMemberTypeAtOffset(DerivedTy->getBaseType().resolve(EmptyMap),
                                    Offset, Size);
    case llvm::dwarf::DW_TAG_member:
      return FindMemberTypeAtOffset(DerivedTy->getBaseType().resolve(EmptyMap),
                                    Offset, Size);
    case llvm::dwarf::DW_TAG_subroutine_type:
      // ignore member functions.
      return nullptr;
    }
  } else if (auto *CompositeTy = llvm::dyn_cast<llvm::DICompositeType>(Ty)) {
    switch (CompositeTy->getTag()) {
    default:
      assert(!"Unhandled DICompositeType");
      return nullptr;
    case llvm::dwarf::DW_TAG_array_type:
      return nullptr;
    case llvm::dwarf::DW_TAG_structure_type:
    case llvm::dwarf::DW_TAG_class_type:
      return FindStructMemberTypeAtOffset(CompositeTy, Offset, Size);
    case llvm::dwarf::DW_TAG_enumeration_type:
      return nullptr;
    }
  } else if (auto *BasicTy = llvm::dyn_cast<llvm::DIBasicType>(Ty)) {
    if (Offset == 0 && Ty->getSizeInBits() == Size) {
      return BasicTy;
    }
  }

  assert(!"Unhandled DIType");
  return nullptr;
}

// SortMembers traverses all of Ty's members and returns them sorted
// by their offset from Ty's start. Returns true if the function succeeds
// and false otherwise.
static bool
SortMembers(llvm::DICompositeType *Ty,
            std::map<OffsetInBits, llvm::DIDerivedType *> *SortedMembers) {
  auto Elements = Ty->getElements();
  if (Elements.begin() == Elements.end()) {
    return false;
  }
  for (auto *Element : Elements) {
    switch (Element->getTag()) {
    case llvm::dwarf::DW_TAG_member: {
      if (auto *Member = llvm::dyn_cast<llvm::DIDerivedType>(Element)) {
        if (Member->getSizeInBits()) {
          auto it = SortedMembers->emplace(
              std::make_pair(Member->getOffsetInBits(), Member));
          (void)it;
          assert(it.second &&
                 "Invalid DIStructType"
                 " - members with the same offset -- are unions possible?");
        }
        break;
      }
      assert(!"member is not a Member");
      return false;
    }
    case llvm::dwarf::DW_TAG_subprogram: {
      if (isa<llvm::DISubprogram>(Element)) {
        continue;
      }
      assert(!"DISubprogram not understood");
      return false;
    }
    case llvm::dwarf::DW_TAG_inheritance: {
      if (auto *Member = llvm::dyn_cast<llvm::DIDerivedType>(Element)) {
        auto it = SortedMembers->emplace(
            std::make_pair(Member->getOffsetInBits(), Member));
        (void)it;
        assert(it.second &&
               "Invalid DIStructType"
               " - members with the same offset -- are unions possible?");
      }
      continue;
    }
    default:
      assert(!"Unhandled field type in DIStructType");
      return false;
    }
  }
  return true;
}

static bool IsResourceObject(llvm::DIDerivedType *DT) {
  const llvm::DITypeIdentifierMap EmptyMap;
  auto *BT = DT->getBaseType().resolve(EmptyMap);
  if (auto *CompositeTy = llvm::dyn_cast<llvm::DICompositeType>(BT)) {
    // Resource variables (e.g. TextureCube) are composite types but have no
    // elements:
    if (CompositeTy->getElements().begin() ==
        CompositeTy->getElements().end()) {
      auto name = CompositeTy->getName();
      auto openTemplateListMarker = name.find_first_of('<');
      if (openTemplateListMarker != llvm::StringRef::npos) {
        auto hlslType = name.substr(0, openTemplateListMarker);
        for (int i = static_cast<int>(hlsl::DXIL::ResourceKind::Invalid) + 1;
             i < static_cast<int>(hlsl::DXIL::ResourceKind::NumEntries); ++i) {
          if (hlslType == hlsl::GetResourceKindName(
                              static_cast<hlsl::DXIL::ResourceKind>(i))) {
            return true;
          }
        }
      }
    }
  }
  return false;
}

static llvm::DIType *FindStructMemberTypeAtOffset(llvm::DICompositeType *Ty,
                                                  uint64_t Offset,
                                                  uint64_t Size) {
  std::map<OffsetInBits, llvm::DIDerivedType *> SortedMembers;
  if (!SortMembers(Ty, &SortedMembers)) {
    return Ty;
  }

  const llvm::DITypeIdentifierMap EmptyMap;

  for (auto &member : SortedMembers) {
    // "Inheritance" is a member of a composite type, but has size of zero.
    // Therefore, we must descend the hierarchy once to find an actual type.
    llvm::DIType *memberType = member.second;
    if (memberType->getTag() == llvm::dwarf::DW_TAG_inheritance) {
      memberType = member.second->getBaseType().resolve(EmptyMap);
    }
    if (Offset >= member.first &&
        Offset < member.first + memberType->getSizeInBits()) {
      uint64_t OffsetIntoThisType = Offset - member.first;
      return FindMemberTypeAtOffset(memberType, OffsetIntoThisType, Size);
    }
  }

  // Structure resources are expected to fail this (they have no real
  // meaning in storage)
  if (SortedMembers.size() == 1) {
    switch (SortedMembers.begin()->second->getTag()) {
    case llvm::dwarf::DW_TAG_structure_type:
    case llvm::dwarf::DW_TAG_class_type:
      if (IsResourceObject(SortedMembers.begin()->second)) {
        return nullptr;
      }
    }
  }
#ifdef VALUE_TO_DECLARE_LOGGING
  VALUE_TO_DECLARE_LOG(
      "Didn't find a member that straddles the sought type. Container:");
  {
    ScopedIndenter indent;
    Ty->dump();
    DumpFullType(Ty);
  }
  VALUE_TO_DECLARE_LOG(
      "Sought type is at offset %d size %d. Members and offsets:", Offset,
      Size);
  {
    ScopedIndenter indent;
    for (auto const &member : SortedMembers) {
      member.second->dump();
      LogPartialLine("Offset %d (size %d): ", member.first,
                     member.second->getSizeInBits());
      DumpFullType(member.second);
    }
  }
#endif
  assert(!"Didn't find a member that straddles the sought type");
  return nullptr;
}

static bool IsDITypePointer(DIType *DTy,
                            const llvm::DITypeIdentifierMap &EmptyMap) {
  DIDerivedType *DerivedTy = dyn_cast<DIDerivedType>(DTy);
  if (!DerivedTy)
    return false;
  switch (DerivedTy->getTag()) {
  case llvm::dwarf::DW_TAG_pointer_type:
    return true;
  case llvm::dwarf::DW_TAG_typedef:
  case llvm::dwarf::DW_TAG_const_type:
  case llvm::dwarf::DW_TAG_restrict_type:
  case llvm::dwarf::DW_TAG_reference_type:
    return IsDITypePointer(DerivedTy->getBaseType().resolve(EmptyMap),
                           EmptyMap);
  }
  return false;
}

void DxilDbgValueToDbgDeclare::handleDbgValue(llvm::Module &M,
                                              llvm::DbgValueInst *DbgValue) {
  VALUE_TO_DECLARE_LOG("DbgValue named %s", DbgValue->getName().str().c_str());

  llvm::DIVariable *Variable = DbgValue->getVariable();
  if (Variable != nullptr) {
    VALUE_TO_DECLARE_LOG("... DbgValue referred to variable named %s",
                         Variable->getName().str().c_str());
  } else {
    VALUE_TO_DECLARE_LOG("... variable was null too");
  }

  llvm::Value *ValueFromDbgInst = DbgValue->getValue();
  if (ValueFromDbgInst == nullptr) {
    // The metadata contained a null Value, so we ignore it. This
    // seems to be a dxcompiler bug.
    VALUE_TO_DECLARE_LOG("...Null value!");
    return;
  }

  const llvm::DITypeIdentifierMap EmptyMap;
  llvm::DIType *Ty = Variable->getType().resolve(EmptyMap);
  if (Ty == nullptr) {
    return;
  }

  if (llvm::isa<llvm::PointerType>(ValueFromDbgInst->getType())) {
    // Safeguard: If the type is not a pointer type, then this is
    // dbg.value directly pointing to a memory location instead of
    // a value.
    if (!IsDITypePointer(Ty, EmptyMap)) {
      // We only know how to handle AllocaInsts for now
      if (!isa<AllocaInst>(ValueFromDbgInst)) {
        VALUE_TO_DECLARE_LOG(
            "... variable had pointer type, but is not an alloca.");
        return;
      }

      IRBuilder<> B(DbgValue->getNextNode());
      ValueFromDbgInst = B.CreateLoad(ValueFromDbgInst);
    }
  }

  // Members' "base type" is actually the containing aggregate's type.
  // To find the actual type of the variable, we must descend the
  // container's type hierarchy to find the type at the expected
  // offset/size.
  if (auto *DerivedTy = llvm::dyn_cast<llvm::DIDerivedType>(Ty)) {
    const llvm::DITypeIdentifierMap EmptyMap;
    switch (DerivedTy->getTag()) {
    case llvm::dwarf::DW_TAG_member: {
      Ty = FindMemberTypeAtOffset(DerivedTy->getBaseType().resolve(EmptyMap),
                                  DerivedTy->getOffsetInBits(),
                                  DerivedTy->getSizeInBits());
      if (Ty == nullptr) {
        return;
      }
    } break;
    }
  }

  auto &Register = m_Registers[Variable];
  if (Register == nullptr) {
    Register.reset(new VariableRegisters(
        DbgValue->getDebugLoc(),
        DbgValue->getParent()->getParent()->getEntryBlock().begin(), Variable,
        Ty, &M));
  }

  // Convert the offset from DbgValue's expression to a packed
  // offset, which we'll need in order to determine the (packed)
  // offset of each scalar Value in DbgValue.
  llvm::DIExpression *expression = DbgValue->getExpression();
  const OffsetInBits AlignedOffsetFromVar =
      GetAlignedOffsetFromDIExpression(expression);
  OffsetInBits PackedOffsetFromVar;
  const OffsetManager &Offsets = Register->GetOffsetManager();
  if (!Offsets.GetPackedOffsetFromAlignedOffset(AlignedOffsetFromVar,
                                                &PackedOffsetFromVar)) {
    // todo: output geometry for GS
    return;
  }

  const OffsetInBits InitialOffset = PackedOffsetFromVar;
  auto *insertPt = llvm::dyn_cast<llvm::Instruction>(ValueFromDbgInst);
  if (insertPt != nullptr && !llvm::isa<TerminatorInst>(insertPt)) {
    insertPt = insertPt->getNextNode();
    // Drivers may crash if phi nodes aren't always at the top of a block,
    // so we must skip over them before inserting instructions.
    while (llvm::isa<llvm::PHINode>(insertPt)) {
      insertPt = insertPt->getNextNode();
    }

    if (insertPt != nullptr) {
      llvm::IRBuilder<> B(insertPt);
      B.SetCurrentDebugLocation(llvm::DebugLoc());

      auto *Zero = B.getInt32(0);

      // Now traverse a list of pairs {Scalar Value, InitialOffset +
      // Offset}. InitialOffset is the offset from DbgValue's expression
      // (i.e., the offset from the Variable's start), and Offset is the
      // Scalar Value's packed offset from DbgValue's value.
      for (const ValueAndOffset &VO :
           SplitValue(ValueFromDbgInst, InitialOffset, B)) {

        OffsetInBits AlignedOffset;
        if (!Offsets.GetAlignedOffsetFromPackedOffset(VO.m_PackedOffset,
                                                      &AlignedOffset)) {
          continue;
        }

        auto *AllocaInst = Register->GetRegisterForAlignedOffset(AlignedOffset);
        if (AllocaInst == nullptr) {
          assert(!"Failed to find alloca for var[offset]");
          continue;
        }

        if (AllocaInst->getAllocatedType()->getArrayElementType() ==
            VO.m_V->getType()) {
          auto *GEP = B.CreateGEP(AllocaInst, {Zero, Zero});
          B.CreateStore(VO.m_V, GEP);
        }
      }
    }
  }
}

struct GlobalVariableAndStorage {
  llvm::DIGlobalVariable *DIGV;
  OffsetInBits Offset;
};

GlobalVariableAndStorage
GetOffsetFromGlobalVariable(llvm::StringRef name,
                            GlobalStorageMap &GlobalEmbeddedArrayStorage) {
  GlobalVariableAndStorage ret{};
  for (auto &Variable : GlobalEmbeddedArrayStorage) {
    for (auto &Storage : Variable.second.ArrayElementStorage) {
      if (llvm::StringRef(Storage.Name).equals(name)) {
        ret.DIGV = Variable.first;
        ret.Offset = Storage.Offset;
        return ret;
      }
    }
  }
  return ret;
}

bool DxilDbgValueToDbgDeclare::handleStoreIfDestIsGlobal(
    llvm::Module &M, GlobalStorageMap &GlobalEmbeddedArrayStorage,
    llvm::StoreInst *Store) {
  if (Store->getDebugLoc()) {
    llvm::Value *V = Store->getPointerOperand();
    std::string MemberName;
    if (auto *Constant = llvm::dyn_cast<llvm::ConstantExpr>(V)) {
      ScopedInstruction asInstr(Constant->getAsInstruction());
      if (auto *asGEP =
              llvm::dyn_cast<llvm::GetElementPtrInst>(asInstr.Get())) {
        // We are only interested in the case of basic types within an array
        // because the PIX debug instrumentation operates at that level.
        // Aggregate members will have been descended through to produce
        // their own entries in the GlobalStorageMap. Consequently, we're
        // only interested in the GEP's index into the array. Any deeper
        // indexing in the GEP will be for embedded aggregates. The three
        // operands in such a GEP mean:
        //    0 = the pointer
        //    1 = dereference the pointer (expected to be constant int zero)
        //    2 = the index into the array
        if (asGEP->getNumOperands() == 3 &&
            llvm::isa<ConstantInt>(asGEP->getOperand(1)) &&
            llvm::dyn_cast<ConstantInt>(asGEP->getOperand(1))
                    ->getLimitedValue() == 0) {
          // TODO: The case where this index is not a constant int
          // (Needs changes to the allocas generated elsewhere in this
          // pass.)
          if (auto *arrayIndexAsConstInt =
                  llvm::dyn_cast<ConstantInt>(asGEP->getOperand(2))) {
            int MemberIndex = arrayIndexAsConstInt->getLimitedValue();
            MemberName = std::string(asGEP->getPointerOperand()->getName()) +
                         "." + std::to_string(MemberIndex);
          }
        }
      }
    } else {
      MemberName = V->getName();
    }
    if (!MemberName.empty()) {
      auto Storage =
          GetOffsetFromGlobalVariable(MemberName, GlobalEmbeddedArrayStorage);
      if (Storage.DIGV != nullptr) {
        llvm::DILocalVariable *Variable =
            GlobalEmbeddedArrayStorage[Storage.DIGV]
                .LocalMirrors[Store->getParent()->getParent()];
        if (Variable != nullptr) {
          const llvm::DITypeIdentifierMap EmptyMap;
          llvm::DIType *Ty = Variable->getType().resolve(EmptyMap);
          if (Ty != nullptr) {
            auto &Register = m_Registers[Variable];
            if (Register == nullptr) {
              Register.reset(new VariableRegisters(
                  Store->getDebugLoc(),
                  Store->getParent()->getParent()->getEntryBlock().begin(),
                  Variable, Ty, &M));
            }
            auto *AllocaInst =
                Register->GetRegisterForAlignedOffset(Storage.Offset);
            if (AllocaInst != nullptr) {
              IRBuilder<> B(Store->getNextNode());
              auto *Zero = B.getInt32(0);
              auto *GEP = B.CreateGEP(AllocaInst, {Zero, Zero});
              B.CreateStore(Store->getValueOperand(), GEP);
              return true; // yes, we modified the module
            }
          }
        }
      }
    }
  }
  return false; // no we did not modify the module
}

SizeInBits VariableRegisters::GetVariableSizeInbits(DIVariable *Var) {
  const llvm::DITypeIdentifierMap EmptyMap;
  DIType *Ty = Var->getType().resolve(EmptyMap);
  DIDerivedType *DerivedTy = nullptr;
  if (BaseTypeIfItIsBasicAndLarger(Ty))
    return Ty->getSizeInBits();
  while (Ty && (Ty->getSizeInBits() == 0 &&
                (DerivedTy = dyn_cast<DIDerivedType>(Ty)))) {
    Ty = DerivedTy->getBaseType().resolve(EmptyMap);
  }

  if (!Ty) {
    assert(false &&
           "Unexpected inability to resolve base type with a real size.");
    return 0;
  }
  return Ty->getSizeInBits();
}

llvm::AllocaInst *
VariableRegisters::GetRegisterForAlignedOffset(OffsetInBits Offset) const {
  auto it = m_AlignedOffsetToAlloca.find(Offset);
  if (it == m_AlignedOffsetToAlloca.end()) {
    return nullptr;
  }
  return it->second;
}

VariableRegisters::VariableRegisters(
    llvm::DebugLoc const &dbgLoc,
    llvm::BasicBlock::iterator allocaInsertionPoint, llvm::DIVariable *Variable,
    llvm::DIType *Ty, llvm::Module *M)
    : m_dbgLoc(dbgLoc), m_Variable(Variable), m_B(allocaInsertionPoint),
      m_DbgDeclareFn(
          llvm::Intrinsic::getDeclaration(M, llvm::Intrinsic::dbg_declare)) {
  PopulateAllocaMap(Ty);
  m_Offsets.AlignTo(Ty); // For padding.

  // (min16* types can occupy 16 or 32 bits depending on whether or not they
  // are natively supported. If non-native, the alignment will be 32, but
  // the claimed size will still be 16, hence the "max" here)
  assert(m_Offsets.GetCurrentAlignedOffset() ==
         std::max<uint64_t>(DITypePeelTypeAlias(Ty)->getSizeInBits(),
                            DITypePeelTypeAlias(Ty)->getAlignInBits()));
}

void VariableRegisters::PopulateAllocaMap(llvm::DIType *Ty) {
  VALUE_TO_DECLARE_LOG("PopulateAllocaMap for type tag %d", Ty->getTag());
  const llvm::DITypeIdentifierMap EmptyMap;
  if (auto *DerivedTy = llvm::dyn_cast<llvm::DIDerivedType>(Ty)) {
    switch (DerivedTy->getTag()) {
    default:
      assert(!"Unhandled DIDerivedType");
      m_Offsets.AlignToAndAddUnhandledType(DerivedTy);
      return;
    case llvm::dwarf::DW_TAG_arg_variable: // "this" pointer
    case llvm::dwarf::DW_TAG_pointer_type: // "this" pointer
    case llvm::dwarf::DW_TAG_restrict_type:
    case llvm::dwarf::DW_TAG_reference_type:
    case llvm::dwarf::DW_TAG_const_type:
    case llvm::dwarf::DW_TAG_typedef:
      PopulateAllocaMap(DerivedTy->getBaseType().resolve(EmptyMap));
      return;
    case llvm::dwarf::DW_TAG_member:
      if (auto *baseType = BaseTypeIfItIsBasicAndLarger(DerivedTy))
        PopulateAllocaMap_BasicType(baseType, DerivedTy->getSizeInBits());
      else
        PopulateAllocaMap(DerivedTy->getBaseType().resolve(EmptyMap));
      return;
    case llvm::dwarf::DW_TAG_subroutine_type:
      // ignore member functions.
      return;
    }
  } else if (auto *CompositeTy = llvm::dyn_cast<llvm::DICompositeType>(Ty)) {
    switch (CompositeTy->getTag()) {
    default:
      assert(!"Unhandled DICompositeType");
      m_Offsets.AlignToAndAddUnhandledType(CompositeTy);
      return;
    case llvm::dwarf::DW_TAG_array_type:
      PopulateAllocaMap_ArrayType(CompositeTy);
      return;
    case llvm::dwarf::DW_TAG_structure_type:
    case llvm::dwarf::DW_TAG_class_type:
      PopulateAllocaMap_StructType(CompositeTy);
      return;
    case llvm::dwarf::DW_TAG_enumeration_type: {
      auto *baseType = CompositeTy->getBaseType().resolve(EmptyMap);
      if (baseType != nullptr) {
        PopulateAllocaMap(baseType);
      } else {
        m_Offsets.AlignToAndAddUnhandledType(CompositeTy);
      }
    }
      return;
    }
  } else if (auto *BasicTy = llvm::dyn_cast<llvm::DIBasicType>(Ty)) {
    PopulateAllocaMap_BasicType(BasicTy, 0 /*no size override*/);
    return;
  }

  assert(!"Unhandled DIType");
  m_Offsets.AlignToAndAddUnhandledType(Ty);
}

static llvm::Type *GetLLVMTypeFromDIBasicType(llvm::IRBuilder<> &B,
                                              llvm::DIBasicType *Ty) {
  const SizeInBits Size = Ty->getSizeInBits();

  switch (Ty->getEncoding()) {
  default:
    break;

  case llvm::dwarf::DW_ATE_boolean:
  case llvm::dwarf::DW_ATE_signed:
  case llvm::dwarf::DW_ATE_unsigned:
    switch (Size) {
    case 16:
      return B.getInt16Ty();
    case 32:
      return B.getInt32Ty();
    case 64:
      return B.getInt64Ty();
    }
    break;
  case llvm::dwarf::DW_ATE_float:
    switch (Size) {
    case 16:
      return B.getHalfTy();
    case 32:
      return B.getFloatTy();
    case 64:
      return B.getDoubleTy();
    }
    break;
  }

  return nullptr;
}

void VariableRegisters::PopulateAllocaMap_BasicType(llvm::DIBasicType *Ty,
                                                    unsigned sizeOverride) {
  llvm::Type *AllocaElementTy = GetLLVMTypeFromDIBasicType(m_B, Ty);
  assert(AllocaElementTy != nullptr);
  if (AllocaElementTy == nullptr) {
    return;
  }

  const auto offsets = m_Offsets.Add(Ty, sizeOverride);

  llvm::Type *AllocaTy = llvm::ArrayType::get(AllocaElementTy, 1);
  llvm::AllocaInst *&Alloca = m_AlignedOffsetToAlloca[offsets.Aligned];
  if (Alloca == nullptr) {
    Alloca = m_B.CreateAlloca(AllocaTy, m_B.getInt32(0));
    Alloca->setDebugLoc(llvm::DebugLoc());
  }

  auto *Storage = GetMetadataAsValue(llvm::ValueAsMetadata::get(Alloca));
  auto *Variable = GetMetadataAsValue(m_Variable);
  auto *Expression = GetMetadataAsValue(
      GetDIExpression(Ty, sizeOverride == 0 ? offsets.Aligned : offsets.Packed,
                      GetVariableSizeInbits(m_Variable), sizeOverride));
  auto *DbgDeclare =
      m_B.CreateCall(m_DbgDeclareFn, {Storage, Variable, Expression});
  DbgDeclare->setDebugLoc(m_dbgLoc);
}

static unsigned NumArrayElements(llvm::DICompositeType *Array) {
  if (Array->getElements().size() == 0) {
    return 0;
  }

  unsigned NumElements = 1;
  for (llvm::DINode *N : Array->getElements()) {
    if (auto *Subrange = llvm::dyn_cast<llvm::DISubrange>(N)) {
      NumElements *= Subrange->getCount();
    } else {
      assert(!"Unhandled array element");
      return 0;
    }
  }
  return NumElements;
}

void VariableRegisters::PopulateAllocaMap_ArrayType(llvm::DICompositeType *Ty) {
  unsigned NumElements = NumArrayElements(Ty);
  if (NumElements == 0) {
    m_Offsets.AlignToAndAddUnhandledType(Ty);
    return;
  }

  const SizeInBits ArraySizeInBits = Ty->getSizeInBits();
  (void)ArraySizeInBits;

  const llvm::DITypeIdentifierMap EmptyMap;
  llvm::DIType *ElementTy = Ty->getBaseType().resolve(EmptyMap);
  assert(ArraySizeInBits % NumElements == 0 &&
         " invalid DIArrayType"
         " - Size is not a multiple of NumElements");

  // After aligning the current aligned offset to ElementTy's natural
  // alignment, the current aligned offset must match Ty's offset
  // in bits.
  m_Offsets.AlignTo(ElementTy);

  for (unsigned i = 0; i < NumElements; ++i) {
    // This is only needed if ElementTy's size is not a multiple of
    // its natural alignment.
    m_Offsets.AlignTo(ElementTy);
    PopulateAllocaMap(ElementTy);
  }
}

void VariableRegisters::PopulateAllocaMap_StructType(
    llvm::DICompositeType *Ty) {
  VALUE_TO_DECLARE_LOG("Struct type : %s, size %d", Ty->getName().str().c_str(),
                       Ty->getSizeInBits());
  std::map<OffsetInBits, llvm::DIDerivedType *> SortedMembers;
  if (!SortMembers(Ty, &SortedMembers)) {
    m_Offsets.AlignToAndAddUnhandledType(Ty);
    return;
  }

  m_Offsets.AlignTo(Ty);
  const OffsetInBits StructStart = m_Offsets.GetCurrentAlignedOffset();
  (void)StructStart;
  const llvm::DITypeIdentifierMap EmptyMap;

  for (auto OffsetAndMember : SortedMembers) {
    VALUE_TO_DECLARE_LOG("Member: %s at packed offset %d",
                         OffsetAndMember.second->getName().str().c_str(),
                         OffsetAndMember.first);
    // Align the offsets to the member's type natural alignment. This
    // should always result in the current aligned offset being the
    // same as the member's offset.
    m_Offsets.AlignTo(OffsetAndMember.second);
    if (BaseTypeIfItIsBasicAndLarger(OffsetAndMember.second)) {
      // This is the bitfields case (i.e. a field that is smaller
      // than the type in which it resides). If we were to take
      // the base type, then the information about the member's
      // size would be lost
      PopulateAllocaMap(OffsetAndMember.second);
    } else {
      if (OffsetAndMember.second->getAlignInBits() ==
          OffsetAndMember.second->getSizeInBits()) {
        assert(m_Offsets.GetCurrentAlignedOffset() ==
                   StructStart + OffsetAndMember.first &&
               "Offset mismatch in DIStructType");
      }
      if (IsResourceObject(OffsetAndMember.second)) {
        m_Offsets.AddResourceType(OffsetAndMember.second);
      } else {
        PopulateAllocaMap(
            OffsetAndMember.second->getBaseType().resolve(EmptyMap));
      }
    }
  }
}

// HLSL Change: remove unused function
#if 0
llvm::DILocation *VariableRegisters::GetVariableLocation() const
{
  const unsigned DefaultColumn = 1;
  return llvm::DILocation::get(
      m_B.getContext(),
      m_Variable->getLine(),
      DefaultColumn,
      m_Variable->getScope());
}
#endif

llvm::Value *VariableRegisters::GetMetadataAsValue(llvm::Metadata *M) const {
  return llvm::MetadataAsValue::get(m_B.getContext(), M);
}

llvm::DIExpression *
VariableRegisters::GetDIExpression(llvm::DIType *Ty, OffsetInBits Offset,
                                   SizeInBits ParentSize,
                                   unsigned sizeOverride) const {
  llvm::SmallVector<uint64_t, 3> ExpElements;
  if (Offset != 0 || Ty->getSizeInBits() != ParentSize) {
    ExpElements.emplace_back(llvm::dwarf::DW_OP_bit_piece);
    ExpElements.emplace_back(Offset);
    ExpElements.emplace_back(sizeOverride != 0 ? sizeOverride
                                               : Ty->getSizeInBits());
  }
  return llvm::DIExpression::get(m_B.getContext(), ExpElements);
}

using namespace llvm;

INITIALIZE_PASS(DxilDbgValueToDbgDeclare, DEBUG_TYPE,
                "Converts calls to dbg.value to dbg.declare + stores to new "
                "virtual registers",
                false, false)

ModulePass *llvm::createDxilDbgValueToDbgDeclarePass() {
  return new DxilDbgValueToDbgDeclare();
}
