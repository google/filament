//===-- SpirvModule.h - SPIR-V Module -------------------------*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_SPIRVMODULE_H
#define LLVM_CLANG_SPIRV_SPIRVMODULE_H

#include <vector>

#include "clang/SPIRV/SpirvInstruction.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallVector.h"

namespace clang {
namespace spirv {

class SpirvFunction;
class SpirvVisitor;

struct ExtensionComparisonInfo {
  static inline SpirvExtension *getEmptyKey() { return nullptr; }
  static inline SpirvExtension *getTombstoneKey() { return nullptr; }
  static unsigned getHashValue(const SpirvExtension *ext) {
    return llvm::hash_combine(ext->getExtensionName());
  }
  static bool isEqual(SpirvExtension *LHS, SpirvExtension *RHS) {
    // Either both are null, or both should have the same underlying extension.
    return (LHS == RHS) || (LHS && RHS && *LHS == *RHS);
  }
};

struct DecorationComparisonInfo {
  static inline SpirvDecoration *getEmptyKey() { return nullptr; }
  static inline SpirvDecoration *getTombstoneKey() { return nullptr; }
  static unsigned getHashValue(const SpirvDecoration *decor) {
    return llvm::hash_combine(decor->getTarget(),
                              static_cast<uint32_t>(decor->getDecoration()));
  }
  static bool isEqual(SpirvDecoration *LHS, SpirvDecoration *RHS) {
    // Either both are null, or both should have the same underlying decoration.
    return (LHS == RHS) || (LHS && RHS && *LHS == *RHS);
  }
};

struct CapabilityComparisonInfo {
  static inline SpirvCapability *getEmptyKey() { return nullptr; }
  static inline SpirvCapability *getTombstoneKey() { return nullptr; }
  static unsigned getHashValue(const SpirvCapability *cap) {
    return llvm::hash_combine(static_cast<uint32_t>(cap->getCapability()));
  }
  static bool isEqual(SpirvCapability *LHS, SpirvCapability *RHS) {
    // Either both are null, or both should have the same underlying capability.
    return (LHS == RHS) || (LHS && RHS && *LHS == *RHS);
  }
};

/// The class representing a SPIR-V module in memory.
///
/// A SPIR-V module contains two main parts: instructions for "metadata" (e.g.,
/// required capabilities and used types) and instructions for shader logic.
/// The former consists of the instructions before the function section in
/// SPIR-V logical layout; while the later is what are in the function section.
///
/// The SpirvBuilder class should be used to gradually build up the second part.
/// After the SpirvBuilder completes its tasks, the first part should be filled
/// out by traversing the second part built by the SpirvBuilder.
///
/// This representation is a just a minimal collection of SPIR-V entities;
/// it does not provide much sanity check over the integrity among the enclosed
/// entities, which modifying classes should be responsible for.
class SpirvModule {
public:
  SpirvModule();
  ~SpirvModule();

  // Forbid copy construction and assignment
  SpirvModule(const SpirvModule &) = delete;
  SpirvModule &operator=(const SpirvModule &) = delete;

  // Forbid move construction and assignment
  SpirvModule(SpirvModule &&) = delete;
  SpirvModule &operator=(SpirvModule &&) = delete;

  // Handle SPIR-V module visitors.
  bool invokeVisitor(Visitor *, bool reverseOrder = false);

  // Add a function to the list of module functions.
  void addFunctionToListOfSortedModuleFunctions(SpirvFunction *);

  // Adds the given function to the vector of all discovered functions. Calling
  // this function will not result in emitting the function.
  void addFunction(SpirvFunction *);

  // Add a capability to the list of module capabilities.
  // Returns true if the capability was added.
  // Returns false otherwise (e.g. if the capability already existed).
  bool addCapability(SpirvCapability *cap);

  // Returns true if the capability is in the module.
  bool hasCapability(SpirvCapability &cap);

  // Set the memory model of the module.
  void setMemoryModel(SpirvMemoryModel *model);

  // Increases addressing model requirement for the module:
  // Logical -> Physical32 -> Physical64 -> PhysicalStorageBuffer64.
  // Requires setMemoryModel() to be called first to set the base memory model.
  // Returns true if addressing model was changed.
  bool promoteAddressingModel(spv::AddressingModel addrModel);

  // Add an entry point to the module.
  void addEntryPoint(SpirvEntryPoint *);

  // Returns an existing execution mode instruction that is the same as em if it
  // exists. Return nullptr otherwise.
  SpirvExecutionModeBase *findExecutionMode(SpirvFunction *entryPoint,
                                            spv::ExecutionMode em);

  // Adds an execution mode to the module.
  void addExecutionMode(SpirvExecutionModeBase *em);

  // Adds an extension to the module. Returns true if the extension was added.
  // Returns false otherwise (e.g. if the extension already existed).
  bool addExtension(SpirvExtension *);

  // Adds an extended instruction set to the module.
  void addExtInstSet(SpirvExtInstImport *);

  // Returns the extended instruction set with the given name if already added
  // Returns nullptr otherwise.
  SpirvExtInstImport *getExtInstSet(llvm::StringRef name);

  // Adds a variable to the module.
  void addVariable(SpirvVariable *);

  // Adds a variable to the module immediately before `pos`.
  // If `pos` is not found, `var` is added at the end of the variable list.
  void addVariable(SpirvVariable *var, SpirvInstruction *pos);

  // Adds a decoration to the module.
  void addDecoration(SpirvDecoration *);

  // Adds a constant to the module.
  void addConstant(SpirvConstant *);

  // Adds an Undef to the module.
  void addUndef(SpirvUndef *);

  // Adds given string to the module which will be emitted via OpString.
  void addString(SpirvString *);

  // Adds the debug source to the module.
  void addSource(SpirvSource *);

  // Adds the given debug info instruction to debugInstructions.
  void addDebugInfo(SpirvDebugInstruction *);

  llvm::SmallVector<SpirvDebugInstruction *, 32> &getDebugInfo() {
    return debugInstructions;
  }

  // Access the one DebugCompilationUnit per module
  SpirvDebugCompilationUnit *getDebugCompilationUnit();
  void setDebugCompilationUnit(SpirvDebugCompilationUnit *unit);

  // Adds the given OpModuleProcessed to the module.
  void addModuleProcessed(SpirvModuleProcessed *);

  llvm::ArrayRef<SpirvVariable *> getVariables() const { return variables; }

  llvm::ArrayRef<SpirvEntryPoint *> getEntryPoints() const {
    return entryPoints;
  }

  void setPerVertexInterpMode(bool b) { perVertexInterp = b; }

  bool isPerVertexInterpMode() const { return perVertexInterp; }

private:
  // Use a set for storing capabilities. This will ensure there are no duplicate
  // capabilities. Although the set stores pointers, the provided
  // CapabilityComparisonInfo compares the SpirvCapability objects, not the
  // pointers.
  llvm::SetVector<SpirvCapability *, std::vector<SpirvCapability *>,
                  llvm::DenseSet<SpirvCapability *, CapabilityComparisonInfo>>
      capabilities;

  // Use a set for storing extensions. This will ensure there are no duplicate
  // extensions. Although the set stores pointers, the provided
  // ExtensionComparisonInfo compares the SpirvExtension objects, not the
  // pointers.
  llvm::SetVector<SpirvExtension *, std::vector<SpirvExtension *>,
                  llvm::DenseSet<SpirvExtension *, ExtensionComparisonInfo>>
      extensions;

  llvm::SmallVector<SpirvExtInstImport *, 1> extInstSets;
  SpirvMemoryModel *memoryModel;
  llvm::SmallVector<SpirvEntryPoint *, 1> entryPoints;
  llvm::SmallVector<SpirvExecutionModeBase *, 4> executionModes;
  llvm::SmallVector<SpirvString *, 4> constStrings;
  std::vector<SpirvSource *> sources;
  std::vector<SpirvModuleProcessed *> moduleProcesses;

  // Use a set for storing decoration. This will ensure that we don't apply the
  // same decoration to the same target more than once. Although the set stores
  // pointers, the provided DecorationComparisonInfo compares the
  // SpirvDecoration objects, not the pointers.
  llvm::SetVector<SpirvDecoration *, std::vector<SpirvDecoration *>,
                  llvm::DenseSet<SpirvDecoration *, DecorationComparisonInfo>>
      decorations;

  std::vector<SpirvConstant *> constants;
  std::vector<SpirvUndef *> undefs;
  std::vector<SpirvVariable *> variables;
  // A vector of functions in the module in the order that they should be
  // emitted. The order starts with the entry-point function followed by a
  // depth-first discovery of functions reachable from the entry-point function.
  std::vector<SpirvFunction *> functions;
  // A vector of all functions that have been visited in the AST tree. This
  // vector is not in any particular order, and may contain unused functions.
  llvm::SetVector<SpirvFunction *> allFunctions;

  // Keep all rich DebugInfo instructions.
  llvm::SmallVector<SpirvDebugInstruction *, 32> debugInstructions;

  // There is one debugCompilationUnit per module
  SpirvDebugCompilationUnit *debugCompilationUnit;

  // Whether current module is in pervertex interpolation mode.
  bool perVertexInterp;
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_SPIRV_SPIRVMODULE_H
