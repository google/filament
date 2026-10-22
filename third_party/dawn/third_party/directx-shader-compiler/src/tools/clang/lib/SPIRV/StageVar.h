//===--- StageVar.h - Classes for stage variable information --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_SPIRV_STAGEVAR_H
#define LLVM_CLANG_LIB_SPIRV_STAGEVAR_H

#include "dxc/DXIL/DxilSemantic.h"
#include "dxc/DXIL/DxilSigPoint.h"
#include "clang/AST/Attr.h"
#include "clang/SPIRV/SpirvInstruction.h"

#include <string>

namespace clang {
namespace spirv {

/// A struct containing information about a particular HLSL semantic.
struct SemanticInfo {
  llvm::StringRef str;            ///< The original semantic string
  const hlsl::Semantic *semantic; ///< The unique semantic object
  llvm::StringRef name;           ///< The semantic string without index
  uint32_t index;                 ///< The semantic index
  SourceLocation loc;             ///< Source code location

  bool isValid() const { return semantic != nullptr; }

  inline hlsl::Semantic::Kind getKind() const;
  /// \brief Returns true if this semantic is a SV_Target.
  inline bool isTarget() const;
};

// A struct containing information about location and component decorations.
struct LocationAndComponent {
  uint32_t location;
  uint32_t component;
  bool componentAlignment;
};

/// \brief The class containing HLSL and SPIR-V information about a Vulkan stage
/// (builtin/input/output) variable.
class StageVar {
public:
  inline StageVar(const hlsl::SigPoint *sig, SemanticInfo semaInfo,
                  const VKBuiltInAttr *builtin, QualType astType,
                  LocationAndComponent locAndComponentCount)
      : sigPoint(sig), semanticInfo(std::move(semaInfo)), builtinAttr(builtin),
        type(astType), value(nullptr), isBuiltin(false),
        storageClass(spv::StorageClass::Max), location(nullptr),
        locationAndComponentCount(locAndComponentCount), entryPoint(nullptr),
        locOrBuiltinDecorateAttr(false) {
    isBuiltin = builtinAttr != nullptr;
  }

  const hlsl::SigPoint *getSigPoint() const { return sigPoint; }
  const SemanticInfo &getSemanticInfo() const { return semanticInfo; }
  std::string getSemanticStr() const;

  QualType getAstType() const { return type; }

  SpirvVariable *getSpirvInstr() const { return value; }
  void setSpirvInstr(SpirvVariable *spvInstr) { value = spvInstr; }

  const VKBuiltInAttr *getBuiltInAttr() const { return builtinAttr; }

  bool isSpirvBuitin() const { return isBuiltin; }
  void setIsSpirvBuiltin() { isBuiltin = true; }

  spv::StorageClass getStorageClass() const { return storageClass; }
  void setStorageClass(spv::StorageClass sc) { storageClass = sc; }

  const VKLocationAttr *getLocationAttr() const { return location; }
  void setLocationAttr(const VKLocationAttr *loc) { location = loc; }

  const VKIndexAttr *getIndexAttr() const { return indexAttr; }
  void setIndexAttr(const VKIndexAttr *idx) { indexAttr = idx; }

  uint32_t getLocationCount() const {
    return locationAndComponentCount.location;
  }
  LocationAndComponent getLocationAndComponentCount() const {
    return locationAndComponentCount;
  }

  SpirvFunction *getEntryPoint() const { return entryPoint; }
  void setEntryPoint(SpirvFunction *entry) { entryPoint = entry; }
  bool hasLocOrBuiltinDecorateAttr() const { return locOrBuiltinDecorateAttr; }
  void setIsLocOrBuiltinDecorateAttr() { locOrBuiltinDecorateAttr = true; }

private:
  /// HLSL SigPoint. It uniquely identifies each set of parameters that may be
  /// input or output for each entry point.
  const hlsl::SigPoint *sigPoint;
  /// Information about HLSL semantic string.
  SemanticInfo semanticInfo;
  /// SPIR-V BuiltIn attribute.
  const VKBuiltInAttr *builtinAttr;
  /// The AST QualType.
  QualType type;
  /// SPIR-V instruction.
  SpirvVariable *value;
  /// Indicates whether this stage variable should be a SPIR-V builtin.
  bool isBuiltin;
  /// SPIR-V storage class this stage variable belongs to.
  spv::StorageClass storageClass;
  /// Location assignment if input/output variable.
  const VKLocationAttr *location;
  /// Index assignment if PS output variable
  const VKIndexAttr *indexAttr;
  /// How many locations and components this stage variable takes.
  LocationAndComponent locationAndComponentCount;
  /// Entry point for this stage variable. If this stage variable is not
  /// specific for an entry point e.g., built-in, it must be nullptr.
  SpirvFunction *entryPoint;
  bool locOrBuiltinDecorateAttr;
};

/// \brief The struct containing information of stage variable's location and
/// index. This information will be used to check the duplication of stage
/// variable's location and index.
struct StageVariableLocationInfo {
  SpirvFunction *entryPoint;
  spv::StorageClass sc;
  uint32_t location;
  uint32_t index;

  static inline StageVariableLocationInfo getEmptyKey() {
    return {nullptr, spv::StorageClass::Max, 0, 0};
  }
  static inline StageVariableLocationInfo getTombstoneKey() {
    return {nullptr, spv::StorageClass::Max, 0xffffffff, 0xffffffff};
  }
  static unsigned getHashValue(const StageVariableLocationInfo &Val) {
    return llvm::hash_combine(Val.entryPoint) ^
           llvm::hash_combine(Val.location) ^ llvm::hash_combine(Val.index) ^
           llvm::hash_combine(static_cast<uint32_t>(Val.sc));
  }
  static bool isEqual(const StageVariableLocationInfo &LHS,
                      const StageVariableLocationInfo &RHS) {
    return LHS.entryPoint == RHS.entryPoint && LHS.sc == RHS.sc &&
           LHS.location == RHS.location && LHS.index == RHS.index;
  }
};

} // end namespace spirv
} // end namespace clang

#endif
