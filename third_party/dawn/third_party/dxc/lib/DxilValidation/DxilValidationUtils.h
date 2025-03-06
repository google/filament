///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// DxilValidationUttils.h                                                    //
// Copyright (C) Microsoft Corporation. All rights reserved.                 //
// This file is distributed under the University of Illinois Open Source     //
// License. See LICENSE.TXT for details.                                     //
//                                                                           //
// This file provides utils for validating DXIL.                             //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "dxc/DXIL/DxilConstants.h"
#include "dxc/DXIL/DxilResourceProperties.h"

#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/ModuleSlotTracker.h"

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace llvm;

namespace llvm {
class Module;
class Function;
class DataLayout;
class Metadata;
class Value;
class GlobalVariable;
class Instruction;
class Type;
} // namespace llvm

namespace hlsl {

///////////////////////////////////////////////////////////////////////////////
// Validation rules.
#include "DxilValidation.inc"

const char *GetValidationRuleText(ValidationRule value);

class DxilEntryProps;
class DxilModule;
class DxilResourceBase;
class DxilSignatureElement;

// Save status like output write for entries.
struct EntryStatus {
  bool hasOutputPosition[DXIL::kNumOutputStreams];
  unsigned OutputPositionMask[DXIL::kNumOutputStreams];
  std::vector<unsigned> outputCols;
  std::vector<unsigned> patchConstOrPrimCols;
  bool m_bCoverageIn, m_bInnerCoverageIn;
  bool hasViewID;
  unsigned domainLocSize;
  EntryStatus(DxilEntryProps &entryProps);
};

struct ValidationContext {
  bool Failed = false;
  Module &M;
  Module *pDebugModule;
  DxilModule &DxilMod;
  const Type *HandleTy;
  const DataLayout &DL;
  DebugLoc LastDebugLocEmit;
  ValidationRule LastRuleEmit;
  std::unordered_set<Function *> entryFuncCallSet;
  std::unordered_set<Function *> patchConstFuncCallSet;
  std::unordered_map<unsigned, bool> UavCounterIncMap;
  std::unordered_map<Value *, unsigned> HandleResIndexMap;
  // TODO: save resource map for each createHandle/createHandleForLib.
  std::unordered_map<Value *, DxilResourceProperties> ResPropMap;
  std::unordered_map<Function *, std::vector<Function *>> PatchConstantFuncMap;
  std::unordered_map<Function *, std::unique_ptr<EntryStatus>> entryStatusMap;
  bool isLibProfile;
  const unsigned kDxilControlFlowHintMDKind;
  const unsigned kDxilPreciseMDKind;
  const unsigned kDxilNonUniformMDKind;
  const unsigned kLLVMLoopMDKind;
  unsigned m_DxilMajor, m_DxilMinor;
  ModuleSlotTracker slotTracker;
  std::unique_ptr<CallGraph> pCallGraph;

  ValidationContext(Module &llvmModule, Module *DebugModule,
                    DxilModule &dxilModule);

  void PropagateResMap(Value *V, DxilResourceBase *Res);
  void BuildResMap();
  bool HasEntryStatus(Function *F);
  EntryStatus &GetEntryStatus(Function *F);
  CallGraph &GetCallGraph();
  DxilResourceProperties GetResourceFromVal(Value *resVal);

  void EmitGlobalVariableFormatError(GlobalVariable *GV, ValidationRule rule,
                                     ArrayRef<StringRef> args);
  // This is the least desirable mechanism, as it has no context.
  void EmitError(ValidationRule rule);

  void FormatRuleText(std::string &ruleText, ArrayRef<StringRef> args);
  void EmitFormatError(ValidationRule rule, ArrayRef<StringRef> args);

  void EmitMetaError(Metadata *Meta, ValidationRule rule);

  // Use this instead of DxilResourceBase::GetGlobalName
  std::string GetResourceName(const hlsl::DxilResourceBase *Res);

  void EmitResourceError(const hlsl::DxilResourceBase *Res,
                         ValidationRule rule);

  void EmitResourceFormatError(const hlsl::DxilResourceBase *Res,
                               ValidationRule rule, ArrayRef<StringRef> args);

  bool IsDebugFunctionCall(Instruction *I);

  Instruction *GetDebugInstr(Instruction *I);

  // Emit Error or note on instruction `I` with `Msg`.
  // If `isError` is true, `Rule` may omit repeated errors
  void EmitInstrDiagMsg(Instruction *I, ValidationRule Rule, std::string Msg,
                        bool isError = true);
  void EmitInstrError(Instruction *I, ValidationRule rule);

  void EmitInstrNote(Instruction *I, std::string Msg);

  void EmitInstrFormatError(Instruction *I, ValidationRule rule,
                            ArrayRef<StringRef> args);

  void EmitSignatureError(DxilSignatureElement *SE, ValidationRule rule);

  void EmitTypeError(Type *Ty, ValidationRule rule);

  void EmitFnError(Function *F, ValidationRule rule);

  void EmitFnFormatError(Function *F, ValidationRule rule,
                         ArrayRef<StringRef> args);

  void EmitFnAttributeError(Function *F, StringRef Kind, StringRef Value);
};

uint32_t ValidateDxilModule(llvm::Module *pModule, llvm::Module *pDebugModule);
} // namespace hlsl
