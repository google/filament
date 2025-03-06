//===-- Analysis.cpp ------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "llvm-c/Analysis.h"
#include "llvm-c/Initialization.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/InitializePasses.h"
#include "llvm/PassRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include <cstring>

using namespace llvm;

// HLSL Change Begin - Windows doesn't support __attribute__((used)) so these methods
// need to be forcibly bound or they could be stripped at build time.
#if defined(_MSC_VER) && (!defined(NDEBUG) || defined(LLVM_ENABLE_DUMP))
#pragma optimize("", off)
void BindDumpMethods() {
  // Pin LLVM dump methods.
  void (__thiscall Module::*pfnModuleDump)() const = &Module::dump;
  (void)pfnModuleDump;
  void (__thiscall Type::*pfnTypeDump)() const = &Type::dump;
  (void)pfnTypeDump;
  void (__thiscall Function::*pfnViewCFGOnly)() const = &Function::viewCFGOnly;
  (void)pfnViewCFGOnly;
}
#pragma optimize("", on)
#define HLSL_BIND_DUMP_METHODS BindDumpMethods();
#else
#define HLSL_BIND_DUMP_METHODS
#endif
// HLSL Change End

/// initializeAnalysis - Initialize all passes linked into the Analysis library.
void llvm::initializeAnalysis(PassRegistry &Registry) {
  initializeAliasAnalysisAnalysisGroup(Registry);
  initializeAliasAnalysisCounterPass(Registry);
  initializeAAEvalPass(Registry);
  initializeAliasDebuggerPass(Registry);
  initializeAliasSetPrinterPass(Registry);
  initializeNoAAPass(Registry);
  initializeBasicAliasAnalysisPass(Registry);
  initializeBlockFrequencyInfoPass(Registry);
  initializeBranchProbabilityInfoPass(Registry);
  initializeCostModelAnalysisPass(Registry);
  initializeCFGViewerPass(Registry);
  initializeCFGPrinterPass(Registry);
  initializeCFGOnlyViewerPass(Registry);
  initializeCFGOnlyPrinterPass(Registry);
  initializeCFLAliasAnalysisPass(Registry);
  initializeDependenceAnalysisPass(Registry);
  initializeDelinearizationPass(Registry);
  initializeDivergenceAnalysisPass(Registry);
  initializeDominanceFrontierPass(Registry);
  initializeDomViewerPass(Registry);
  initializeDomPrinterPass(Registry);
  initializeDomOnlyViewerPass(Registry);
  initializePostDomViewerPass(Registry);
  initializeDomOnlyPrinterPass(Registry);
  initializePostDomPrinterPass(Registry);
  initializePostDomOnlyViewerPass(Registry);
  initializePostDomOnlyPrinterPass(Registry);
  initializeIVUsersPass(Registry);
  initializeInstCountPass(Registry);
  initializeIntervalPartitionPass(Registry);
  initializeLazyValueInfoPass(Registry);
  initializeLibCallAliasAnalysisPass(Registry);
  initializeLintPass(Registry);
  initializeLoopInfoWrapperPassPass(Registry);
  initializeMemDepPrinterPass(Registry);
  initializeMemDerefPrinterPass(Registry);
  initializeMemoryDependenceAnalysisPass(Registry);
  initializeModuleDebugInfoPrinterPass(Registry);
  initializePostDominatorTreePass(Registry);
  initializeRegionInfoPassPass(Registry);
  initializeRegionViewerPass(Registry);
  initializeRegionPrinterPass(Registry);
  initializeRegionOnlyViewerPass(Registry);
  initializeRegionOnlyPrinterPass(Registry);
  initializeScalarEvolutionPass(Registry);
  initializeScalarEvolutionAliasAnalysisPass(Registry);
  initializeTargetTransformInfoWrapperPassPass(Registry);
  initializeTypeBasedAliasAnalysisPass(Registry);
  initializeScopedNoAliasAAPass(Registry);

  HLSL_BIND_DUMP_METHODS // HLSL Change - Force binding dump methods.
}

void LLVMInitializeAnalysis(LLVMPassRegistryRef R) {
  initializeAnalysis(*unwrap(R));
}

LLVMBool LLVMVerifyModule(LLVMModuleRef M, LLVMVerifierFailureAction Action,
                          char **OutMessages) {
  raw_ostream *DebugOS = Action != LLVMReturnStatusAction ? &errs() : nullptr;
  std::string Messages;
  raw_string_ostream MsgsOS(Messages);

  LLVMBool Result = verifyModule(*unwrap(M), OutMessages ? &MsgsOS : DebugOS);

  // Duplicate the output to stderr.
  if (DebugOS && OutMessages)
    *DebugOS << MsgsOS.str();

  if (Action == LLVMAbortProcessAction && Result)
    report_fatal_error("Broken module found, compilation aborted!");

  if (OutMessages)
    *OutMessages = _strdup(MsgsOS.str().c_str()); // HLSL Change for strdup

  return Result;
}

LLVMBool LLVMVerifyFunction(LLVMValueRef Fn, LLVMVerifierFailureAction Action) {
  LLVMBool Result = verifyFunction(
      *unwrap<Function>(Fn), Action != LLVMReturnStatusAction ? &errs()
                                                              : nullptr);

  if (Action == LLVMAbortProcessAction && Result)
    report_fatal_error("Broken function found, compilation aborted!");

  return Result;
}

void LLVMViewFunctionCFG(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFG();
}

void LLVMViewFunctionCFGOnly(LLVMValueRef Fn) {
  Function *F = unwrap<Function>(Fn);
  F->viewCFGOnly();
}
