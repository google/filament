//===- NewPMDriver.cpp - Driver for opt with new PM -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
///
/// This file is just a split of the code that logically belongs in opt.cpp but
/// that includes the new pass manager headers.
///
//===----------------------------------------------------------------------===//

#include "NewPMDriver.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Target/TargetMachine.h"

using namespace llvm;
using namespace opt_tool;

static cl::opt<bool>
    DebugPM("debug-pass-manager", cl::Hidden,
            cl::desc("Print pass management debugging information"));

bool llvm::runPassPipeline(StringRef Arg0, LLVMContext &Context, Module &M,
                           TargetMachine *TM, tool_output_file *Out,
                           StringRef PassPipeline, OutputKind OK,
                           VerifierKind VK,
                           bool ShouldPreserveAssemblyUseListOrder,
                           bool ShouldPreserveBitcodeUseListOrder) {
  PassBuilder PB(TM);

  FunctionAnalysisManager FAM(DebugPM);
  CGSCCAnalysisManager CGAM(DebugPM);
  ModuleAnalysisManager MAM(DebugPM);

  // Register all the basic analyses with the managers.
  PB.registerModuleAnalyses(MAM);
  PB.registerCGSCCAnalyses(CGAM);
  PB.registerFunctionAnalyses(FAM);

  // Cross register the analysis managers through their proxies.
  MAM.registerPass(FunctionAnalysisManagerModuleProxy(FAM));
  MAM.registerPass(CGSCCAnalysisManagerModuleProxy(CGAM));
  CGAM.registerPass(FunctionAnalysisManagerCGSCCProxy(FAM));
  CGAM.registerPass(ModuleAnalysisManagerCGSCCProxy(MAM));
  FAM.registerPass(CGSCCAnalysisManagerFunctionProxy(CGAM));
  FAM.registerPass(ModuleAnalysisManagerFunctionProxy(MAM));

  ModulePassManager MPM(DebugPM);
  if (VK > VK_NoVerifier)
    MPM.addPass(VerifierPass());

  if (!PB.parsePassPipeline(MPM, PassPipeline, VK == VK_VerifyEachPass,
                            DebugPM)) {
    errs() << Arg0 << ": unable to parse pass pipeline description.\n";
    return false;
  }

  if (VK > VK_NoVerifier)
    MPM.addPass(VerifierPass());

  // Add any relevant output pass at the end of the pipeline.
  switch (OK) {
  case OK_NoOutput:
    break; // No output pass needed.
  case OK_OutputAssembly:
    MPM.addPass(
        PrintModulePass(Out->os(), "", ShouldPreserveAssemblyUseListOrder));
    break;
  case OK_OutputBitcode:
    MPM.addPass(
        BitcodeWriterPass(Out->os(), ShouldPreserveBitcodeUseListOrder));
    break;
  }

  // Before executing passes, print the final values of the LLVM options.
  cl::PrintOptionValues();

  // Now that we have all of the passes ready, run them.
  MPM.run(M, &MAM);

  // Declare success.
  if (OK != OK_NoOutput)
    Out->keep();
  return true;
}
