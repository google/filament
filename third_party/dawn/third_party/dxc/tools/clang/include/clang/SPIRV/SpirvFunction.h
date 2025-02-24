//===-- SpirvFunction.h - SPIR-V Function ---------------------*- C++ -*---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_CLANG_SPIRV_SPIRVFUNCTION_H
#define LLVM_CLANG_SPIRV_SPIRVFUNCTION_H

#include <vector>

#include "clang/SPIRV/SpirvBasicBlock.h"
#include "clang/SPIRV/SpirvInstruction.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"

namespace clang {
namespace spirv {

class SpirvVisitor;

/// The class representing a SPIR-V function in memory.
class SpirvFunction {
public:
  SpirvFunction(QualType astReturnType, SourceLocation,
                llvm::StringRef name = "", bool precise = false,
                bool noInline = false);

  ~SpirvFunction();

  // Forbid copy construction and assignment
  SpirvFunction(const SpirvFunction &) = delete;
  SpirvFunction &operator=(const SpirvFunction &) = delete;

  // Forbid move construction and assignment
  SpirvFunction(SpirvFunction &&) = delete;
  SpirvFunction &operator=(SpirvFunction &&) = delete;

  // Handle SPIR-V function visitors.
  bool invokeVisitor(Visitor *, bool reverseOrder = false);

  uint32_t getResultId() const { return functionId; }
  void setResultId(uint32_t id) { functionId = id; }

  // Sets the lowered (SPIR-V) return type.
  void setReturnType(const SpirvType *type) { returnType = type; }
  // Returns the lowered (SPIR-V) return type.
  const SpirvType *getReturnType() const { return returnType; }

  // Sets the function AST return type
  void setAstReturnType(QualType type) { astReturnType = type; }
  // Gets the function AST return type
  QualType getAstReturnType() const { return astReturnType; }

  // Gets the vector of parameters.
  llvm::SmallVector<SpirvFunctionParameter *, 8> getParameters() const {
    return parameters;
  }

  // Gets the vector of variables.
  std::vector<SpirvVariable *> getVariables() { return variables; }

  // Sets the SPIR-V type of the function
  void setFunctionType(SpirvType *type) { fnType = type; }
  // Returns the SPIR-V type of the function
  SpirvType *getFunctionType() const { return fnType; }

  // Store that the return type is at relaxed precision.
  void setRelaxedPrecision() { relaxedPrecision = true; }
  // Returns whether the return type has relaxed precision.
  bool isRelaxedPrecision() const { return relaxedPrecision; }

  // Store that the return value is precise.
  void setPrecise(bool p = true) { precise = p; }
  // Store that the function should not be inlined.
  void setNoInline(bool n = true) { noInline = n; }
  // Returns whether the return value is precise.
  bool isPrecise() const { return precise; }
  // Returns whether the function is marked as no inline
  bool isNoInline() const { return noInline; }

  void setSourceLocation(SourceLocation loc) { functionLoc = loc; }
  SourceLocation getSourceLocation() const { return functionLoc; }

  void setFunctionName(llvm::StringRef name) { functionName = name; }
  llvm::StringRef getFunctionName() const { return functionName; }

  void addParameter(SpirvFunctionParameter *);
  void addParameterDebugDeclare(SpirvDebugDeclare *inst) {
    debugDeclares.push_back(inst);
  }
  void addVariable(SpirvVariable *);
  void addBasicBlock(SpirvBasicBlock *);

  /// Adds the given instruction as the first instruction of this SPIR-V
  /// function body.
  void addFirstInstruction(SpirvInstruction *inst) {
    assert(basicBlocks.size() != 0);
    basicBlocks[0]->addFirstInstruction(inst);
  }

  /// Adds instructions to a cache array.
  void addToInstructionCache(SpirvInstruction *inst) {
    instructionsCache.push_back(inst);
  }

  /// Adds cached instructions to the front of current function.
  void addInstrCacheToFront() {
    int cacheSize = instructionsCache.size();
    for (int i = 0; i < cacheSize; i++) {
      auto *inst = instructionsCache.back();
      addFirstInstruction(inst);
      instructionsCache.pop_back();
    }
    instructionsCache.clear();
  }
  /// Legalization-specific code
  ///
  /// Note: the following methods are used for properly handling aliasing.
  ///
  /// TODO: Clean up aliasing and try to move it to a separate pass.
  void setConstainsAliasComponent(bool isAlias) { containsAlias = isAlias; }
  bool constainsAliasComponent() { return containsAlias; }
  void setRValue() { rvalue = true; }
  bool isRValue() { return rvalue; }

  /// Get/set DebugScope for this function.
  SpirvDebugScope *getDebugScope() const { return debugScope; }
  void setDebugScope(SpirvDebugScope *scope) { debugScope = scope; }

  bool isEntryFunctionWrapper() const { return isWrapperOfEntry; }
  void setEntryFunctionWrapper() { isWrapperOfEntry = true; }

  /// Returns true if this is a member function of a struct or class.
  bool isMemberFunction() const {
    if (parameters.empty())
      return false;
    return parameters[0]->getDebugName() == "param.this";
  }

  /// Get or set a record for relationship between
  /// a function parameter and variable within current function.
  void addFuncParamVarEntry(SpirvInstruction *v, SpirvInstruction *p) {
    funcVarParamMap[v] = p;
  }
  SpirvInstruction *getMappedFuncParam(SpirvInstruction *v) {
    return funcVarParamMap.lookup(v);
  }

private:
  uint32_t functionId;         ///< This function's <result-id>
  QualType astReturnType;      ///< The return type
  const SpirvType *returnType; ///< The lowered return type
  SpirvType *fnType;           ///< The SPIR-V function type
  bool relaxedPrecision; ///< Whether the return type is at relaxed precision
  bool precise;          ///< Whether the return value is 'precise'
  bool noInline;         ///< The function is marked as no inline
  ///< An instructions cache vector. Would be used to help insert instructions
  ///< at the beginning of a function.
  std::vector<SpirvInstruction *> instructionsCache;

  /// Legalization-specific code
  ///
  /// Note: the following two member variables are currently needed in order to
  /// support aliasing for functions.
  ///
  /// TODO: Clean up aliasing and try to move it to a separate pass.
  bool containsAlias; ///< Whether function return type is aliased
  bool rvalue;        ///< Whether the return value is an rvalue

  SourceLocation functionLoc; ///< Location in source code
  std::string functionName;   ///< This function's name

  /// Parameters to this function.
  llvm::SmallVector<SpirvFunctionParameter *, 8> parameters;

  /// Variables defined in this function.
  ///
  /// Local variables inside a function should be defined at the beginning
  /// of the entry basic block. This serves as a temporary place for holding
  /// these variables.
  std::vector<SpirvVariable *> variables;

  /// Basic blocks inside this function.
  std::vector<SpirvBasicBlock *> basicBlocks;

  /// True if it is a wrapper function for an entry point function.
  bool isWrapperOfEntry;

  /// DebugScope that groups all instructions in this function.
  SpirvDebugScope *debugScope;

  /// DebugDeclare instructions for parameters to this function.
  llvm::SmallVector<SpirvDebugDeclare *, 8> debugDeclares;

  /// Record relationship between a function parameter and its mapped variable.
  llvm::DenseMap<SpirvInstruction *, SpirvInstruction *> funcVarParamMap;
};

} // end namespace spirv
} // end namespace clang

#endif // LLVM_CLANG_SPIRV_SPIRVFUNCTION_H
