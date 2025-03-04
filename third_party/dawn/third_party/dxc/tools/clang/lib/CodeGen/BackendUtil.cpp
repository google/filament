//===--- BackendUtil.cpp - LLVM Backend Utilities -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "clang/CodeGen/BackendUtil.h"
#include "dxc/HLSL/DxilGenerationPass.h" // HLSL Change
#include "dxc/HLSL/HLMatrixLowerPass.h"  // HLSL Change
#include "dxc/Support/Global.h"          // HLSL Change
#include "dxc/config.h"                  // HLSL Change
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/TargetOptions.h"
#include "clang/Frontend/CodeGenOptions.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/Utils.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Bitcode/BitcodeWriterPass.h"
#include "llvm/CodeGen/RegAllocRegistry.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/IRPrintingPasses.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TimeProfiler.h" // HLSL Change
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Instrumentation.h"
#include "llvm/Transforms/ObjCARC.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils/SymbolRewriter.h"
#include <cstdio>
#include <memory>

using namespace clang;
using namespace llvm;

namespace {

class EmitAssemblyHelper {
  DiagnosticsEngine &Diags;
  const CodeGenOptions &CodeGenOpts;
  const clang::TargetOptions &TargetOpts;
  const LangOptions &LangOpts;
  Module *TheModule;
  // HLSL Changes Start
  std::string CodeGenPassesConfig;
  std::string PerModulePassesConfig;
  std::string PerFunctionPassesConfig;
  mutable llvm::raw_string_ostream CodeGenPassesConfigOS;
  mutable llvm::raw_string_ostream PerModulePassesConfigOS;
  mutable llvm::raw_string_ostream PerFunctionPassesConfigOS;
  // HLSL Changes End

  Timer CodeGenerationTime;

  mutable legacy::PassManager *CodeGenPasses;
  mutable legacy::PassManager *PerModulePasses;
  mutable legacy::FunctionPassManager *PerFunctionPasses;

private:
  TargetIRAnalysis getTargetIRAnalysis() const {
    if (TM)
      return TM->getTargetIRAnalysis();

    return TargetIRAnalysis();
  }

  legacy::PassManager *getCodeGenPasses() const {
    if (!CodeGenPasses) {
      CodeGenPasses = new legacy::PassManager();
      CodeGenPasses->TrackPassOS = &CodeGenPassesConfigOS;
      CodeGenPasses->add(
          createTargetTransformInfoWrapperPass(getTargetIRAnalysis()));
    }
    return CodeGenPasses;
  }

  legacy::PassManager *getPerModulePasses() const {
    if (!PerModulePasses) {
      PerModulePasses = new legacy::PassManager();
      PerModulePasses->HLSLPrintBeforeAll =
          this->CodeGenOpts.HLSLPrintBeforeAll;
      PerModulePasses->HLSLPrintBefore = this->CodeGenOpts.HLSLPrintBefore;
      PerModulePasses->HLSLPrintAfterAll = this->CodeGenOpts.HLSLPrintAfterAll;
      PerModulePasses->HLSLPrintAfter = this->CodeGenOpts.HLSLPrintAfter;
      PerModulePasses->TrackPassOS = &PerModulePassesConfigOS;
      PerModulePasses->add(
          createTargetTransformInfoWrapperPass(getTargetIRAnalysis()));
    }
    return PerModulePasses;
  }

  legacy::FunctionPassManager *getPerFunctionPasses() const {
    if (!PerFunctionPasses) {
      PerFunctionPasses = new legacy::FunctionPassManager(TheModule);
      PerFunctionPasses->HLSLPrintBeforeAll =
          this->CodeGenOpts.HLSLPrintBeforeAll;
      PerFunctionPasses->HLSLPrintBefore = this->CodeGenOpts.HLSLPrintBefore;
      PerFunctionPasses->HLSLPrintAfterAll =
          this->CodeGenOpts.HLSLPrintAfterAll;
      PerFunctionPasses->HLSLPrintAfter = this->CodeGenOpts.HLSLPrintAfter;
      PerFunctionPasses->TrackPassOS = &PerFunctionPassesConfigOS;
      PerFunctionPasses->add(
          createTargetTransformInfoWrapperPass(getTargetIRAnalysis()));
    }
    return PerFunctionPasses;
  }

  void CreatePasses();

  /// Generates the TargetMachine.
  /// Returns Null if it is unable to create the target machine.
  /// Some of our clang tests specify triples which are not built
  /// into clang. This is okay because these tests check the generated
  /// IR, and they require DataLayout which depends on the triple.
  /// In this case, we allow this method to fail and not report an error.
  /// When MustCreateTM is used, we print an error if we are unable to load
  /// the requested target.
  TargetMachine *CreateTargetMachine(bool MustCreateTM);

  /// Add passes necessary to emit assembly or LLVM IR.
  ///
  /// \return True on success.
  bool AddEmitPasses(BackendAction Action, raw_pwrite_stream &OS);

public:
  EmitAssemblyHelper(DiagnosticsEngine &_Diags,
                     const CodeGenOptions &CGOpts,
                     const clang::TargetOptions &TOpts,
                     const LangOptions &LOpts,
                     Module *M)
    : Diags(_Diags), CodeGenOpts(CGOpts), TargetOpts(TOpts), LangOpts(LOpts),
      TheModule(M),
      // HLSL Changes Start
      CodeGenPassesConfigOS(CodeGenPassesConfig),
      PerModulePassesConfigOS(PerModulePassesConfig),
      PerFunctionPassesConfigOS(PerFunctionPassesConfig),
      // HLSL Changes End
      CodeGenerationTime("Code Generation Time"),
      CodeGenPasses(nullptr), PerModulePasses(nullptr),
      PerFunctionPasses(nullptr) {}

  ~EmitAssemblyHelper() {
    delete CodeGenPasses;
    delete PerModulePasses;
    delete PerFunctionPasses;
    if (CodeGenOpts.DisableFree)
      BuryPointer(std::move(TM));
  }

  std::unique_ptr<TargetMachine> TM;

  void EmitAssembly(BackendAction Action, raw_pwrite_stream *OS);
};

// We need this wrapper to access LangOpts and CGOpts from extension functions
// that we add to the PassManagerBuilder.
class PassManagerBuilderWrapper : public PassManagerBuilder {
public:
  PassManagerBuilderWrapper(const CodeGenOptions &CGOpts,
                            const LangOptions &LangOpts)
      : PassManagerBuilder(), CGOpts(CGOpts), LangOpts(LangOpts) {}
  const CodeGenOptions &getCGOpts() const { return CGOpts; }
  const LangOptions &getLangOpts() const { return LangOpts; }
private:
  const CodeGenOptions &CGOpts;
  const LangOptions &LangOpts;
};

}

#ifdef MS_ENABLE_OBJCARC // HLSL Change

static void addObjCARCAPElimPass(const PassManagerBuilder &Builder, PassManagerBase &PM) {
  if (Builder.OptLevel > 0)
    PM.add(createObjCARCAPElimPass());
}

static void addObjCARCExpandPass(const PassManagerBuilder &Builder, PassManagerBase &PM) {
  if (Builder.OptLevel > 0)
    PM.add(createObjCARCExpandPass());
}

static void addObjCARCOptPass(const PassManagerBuilder &Builder, PassManagerBase &PM) {
  if (Builder.OptLevel > 0)
    PM.add(createObjCARCOptPass());
}

#endif // MS_ENABLE_OBJCARC - HLSL Change

static void addSampleProfileLoaderPass(const PassManagerBuilder &Builder,
                                       legacy::PassManagerBase &PM) {
  const PassManagerBuilderWrapper &BuilderWrapper =
      static_cast<const PassManagerBuilderWrapper &>(Builder);
  const CodeGenOptions &CGOpts = BuilderWrapper.getCGOpts();
  PM.add(createSampleProfileLoaderPass(CGOpts.SampleProfileFile));
}

static void addAddDiscriminatorsPass(const PassManagerBuilder &Builder,
                                     legacy::PassManagerBase &PM) {
  PM.add(createAddDiscriminatorsPass());
}

#ifdef MS_ENABLE_OBJCARC // HLSL Change
static void addBoundsCheckingPass(const PassManagerBuilder &Builder,
                                    legacy::PassManagerBase &PM) {
  PM.add(createBoundsCheckingPass());
}
#endif // MS_ENABLE_OBJCARC // HLSL Change

#ifdef MS_ENABLE_INSTR // HLSL Change

static void addSanitizerCoveragePass(const PassManagerBuilder &Builder,
                                     legacy::PassManagerBase &PM) {
  const PassManagerBuilderWrapper &BuilderWrapper =
      static_cast<const PassManagerBuilderWrapper&>(Builder);
  const CodeGenOptions &CGOpts = BuilderWrapper.getCGOpts();
  SanitizerCoverageOptions Opts;
  Opts.CoverageType =
      static_cast<SanitizerCoverageOptions::Type>(CGOpts.SanitizeCoverageType);
  Opts.IndirectCalls = CGOpts.SanitizeCoverageIndirectCalls;
  Opts.TraceBB = CGOpts.SanitizeCoverageTraceBB;
  Opts.TraceCmp = CGOpts.SanitizeCoverageTraceCmp;
  Opts.Use8bitCounters = CGOpts.SanitizeCoverage8bitCounters;
  PM.add(createSanitizerCoverageModulePass(Opts));
}

static void addAddressSanitizerPasses(const PassManagerBuilder &Builder,
                                      legacy::PassManagerBase &PM) {
  PM.add(createAddressSanitizerFunctionPass(/*CompileKernel*/false));
  PM.add(createAddressSanitizerModulePass(/*CompileKernel*/false));
}

static void addKernelAddressSanitizerPasses(const PassManagerBuilder &Builder,
                                            legacy::PassManagerBase &PM) {
  PM.add(createAddressSanitizerFunctionPass(/*CompileKernel*/true));
  PM.add(createAddressSanitizerModulePass(/*CompileKernel*/true));
}

static void addMemorySanitizerPass(const PassManagerBuilder &Builder,
                                   legacy::PassManagerBase &PM) {
  const PassManagerBuilderWrapper &BuilderWrapper =
      static_cast<const PassManagerBuilderWrapper&>(Builder);
  const CodeGenOptions &CGOpts = BuilderWrapper.getCGOpts();
  PM.add(createMemorySanitizerPass(CGOpts.SanitizeMemoryTrackOrigins));

  // MemorySanitizer inserts complex instrumentation that mostly follows
  // the logic of the original code, but operates on "shadow" values.
  // It can benefit from re-running some general purpose optimization passes.
  if (Builder.OptLevel > 0) {
    PM.add(createEarlyCSEPass());
    PM.add(createReassociatePass());
    PM.add(createLICMPass());
    PM.add(createGVNPass());
    PM.add(createInstructionCombiningPass());
    PM.add(createDeadStoreEliminationPass());
  }
}

static void addThreadSanitizerPass(const PassManagerBuilder &Builder,
                                   legacy::PassManagerBase &PM) {
  PM.add(createThreadSanitizerPass());
}

static void addDataFlowSanitizerPass(const PassManagerBuilder &Builder,
                                     legacy::PassManagerBase &PM) {
  const PassManagerBuilderWrapper &BuilderWrapper =
      static_cast<const PassManagerBuilderWrapper&>(Builder);
  const LangOptions &LangOpts = BuilderWrapper.getLangOpts();
  PM.add(createDataFlowSanitizerPass(LangOpts.SanitizerBlacklistFiles));
}

#endif // MS_ENABLE_INSTR - HLSL Change

static TargetLibraryInfoImpl *createTLII(llvm::Triple &TargetTriple,
                                         const CodeGenOptions &CodeGenOpts) {
  TargetLibraryInfoImpl *TLII = new TargetLibraryInfoImpl(TargetTriple);
  if (!CodeGenOpts.SimplifyLibCalls)
    TLII->disableAllFunctions();

  switch (CodeGenOpts.getVecLib()) {
  case CodeGenOptions::Accelerate:
    TLII->addVectorizableFunctionsFromVecLib(TargetLibraryInfoImpl::Accelerate);
    break;
  default:
    break;
  }
  return TLII;
}

static void addSymbolRewriterPass(const CodeGenOptions &Opts,
                                  legacy::PassManager *MPM) {
  llvm::SymbolRewriter::RewriteDescriptorList DL;

  llvm::SymbolRewriter::RewriteMapParser MapParser;
  for (const auto &MapFile : Opts.RewriteMapFiles)
    MapParser.parse(MapFile, &DL);

  MPM->add(createRewriteSymbolsPass(DL));
}

void EmitAssemblyHelper::CreatePasses() {
  unsigned OptLevel = CodeGenOpts.OptimizationLevel;
  CodeGenOptions::InliningMethod Inlining = CodeGenOpts.getInlining();

  // Handle disabling of LLVM optimization, where we want to preserve the
  // internal module before any optimization.
  if (CodeGenOpts.DisableLLVMOpts || CodeGenOpts.HLSLHighLevel) { // HLSL Change
    OptLevel = 0;
    // HLSL Change Begins.
    // HLSL always inline.
    if (!LangOpts.HLSL || CodeGenOpts.HLSLHighLevel)
    Inlining = CodeGenOpts.NoInlining;
    // HLSL Change Ends.
  }

  PassManagerBuilderWrapper PMBuilder(CodeGenOpts, LangOpts);
  PMBuilder.OptLevel = OptLevel;
  PMBuilder.SizeLevel = CodeGenOpts.OptimizeSize;
  PMBuilder.BBVectorize = CodeGenOpts.VectorizeBB;
  PMBuilder.SLPVectorize = CodeGenOpts.VectorizeSLP;
  PMBuilder.LoopVectorize = CodeGenOpts.VectorizeLoop;

  // HLSL Change - begin
  PMBuilder.HLSLHighLevel = CodeGenOpts.HLSLHighLevel;
  PMBuilder.HLSLAllowPreserveValues = CodeGenOpts.HLSLAllowPreserveValues;
  PMBuilder.HLSLOnlyWarnOnUnrollFail = CodeGenOpts.HLSLOnlyWarnOnUnrollFail;
  PMBuilder.HLSLExtensionsCodeGen = CodeGenOpts.HLSLExtensionsCodegen.get();
  PMBuilder.HLSLResMayAlias = CodeGenOpts.HLSLResMayAlias;
  PMBuilder.ScanLimit = CodeGenOpts.ScanLimit;

  // Opt toggles
  const hlsl::options::OptimizationToggles &OptToggles =
      CodeGenOpts.HLSLOptimizationToggles;
  PMBuilder.EnableGVN = OptToggles.IsEnabled(hlsl::options::TOGGLE_GVN);
  PMBuilder.HLSLNoSink = !OptToggles.IsEnabled(hlsl::options::TOGGLE_SINK);
  PMBuilder.StructurizeLoopExitsForUnroll = OptToggles.IsEnabled(
      hlsl::options::TOGGLE_STRUCTURIZE_LOOP_EXITS_FOR_UNROLL);
  PMBuilder.HLSLEnableDebugNops = OptToggles.IsEnabled(hlsl::options::TOGGLE_DEBUG_NOPS);
  PMBuilder.HLSLEnableLifetimeMarkers =
      CodeGenOpts.HLSLEnableLifetimeMarkers &&
      OptToggles.IsEnabled(hlsl::options::TOGGLE_LIFETIME_MARKERS);
  PMBuilder.HLSLEnablePartialLifetimeMarkers =
      OptToggles.IsEnabled(hlsl::options::TOGGLE_PARTIAL_LIFETIME_MARKERS);
  PMBuilder.HLSLEnableAggressiveReassociation = OptToggles.IsEnabled(
      hlsl::options::TOGGLE_ENABLE_AGGRESSIVE_REASSOCIATION);
  // HLSL Change - end

  PMBuilder.DisableUnitAtATime = !CodeGenOpts.UnitAtATime;
  PMBuilder.DisableUnrollLoops = !CodeGenOpts.UnrollLoops;
  PMBuilder.MergeFunctions = CodeGenOpts.MergeFunctions;
  PMBuilder.PrepareForLTO = CodeGenOpts.PrepareForLTO;
  PMBuilder.RerollLoops = CodeGenOpts.RerollLoops;

  PMBuilder.addExtension(PassManagerBuilder::EP_EarlyAsPossible,
                         addAddDiscriminatorsPass);

  if (!CodeGenOpts.SampleProfileFile.empty())
    PMBuilder.addExtension(PassManagerBuilder::EP_EarlyAsPossible,
                           addSampleProfileLoaderPass);

  // In ObjC ARC mode, add the main ARC optimization passes.
#ifdef MS_ENABLE_OBJCARC // HLSL Change
  if (LangOpts.ObjCAutoRefCount) {
    PMBuilder.addExtension(PassManagerBuilder::EP_EarlyAsPossible,
                           addObjCARCExpandPass);
    PMBuilder.addExtension(PassManagerBuilder::EP_ModuleOptimizerEarly,
                           addObjCARCAPElimPass);
    PMBuilder.addExtension(PassManagerBuilder::EP_ScalarOptimizerLate,
                           addObjCARCOptPass);
  }

  if (LangOpts.Sanitize.has(SanitizerKind::LocalBounds)) {
    PMBuilder.addExtension(PassManagerBuilder::EP_ScalarOptimizerLate,
                           addBoundsCheckingPass);
    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0,
                           addBoundsCheckingPass);
  }
#endif // MS_ENABLE_OBJCARC - HLSL Change

#ifdef MS_ENABLE_INSTR // HLSL Change

  if (CodeGenOpts.SanitizeCoverageType ||
      CodeGenOpts.SanitizeCoverageIndirectCalls ||
      CodeGenOpts.SanitizeCoverageTraceCmp) {
    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast,
                           addSanitizerCoveragePass);
    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0,
                           addSanitizerCoveragePass);
  }

  if (LangOpts.Sanitize.has(SanitizerKind::Address)) {
    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast,
                           addAddressSanitizerPasses);
    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0,
                           addAddressSanitizerPasses);
  }

  if (LangOpts.Sanitize.has(SanitizerKind::KernelAddress)) {
    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast,
                           addKernelAddressSanitizerPasses);
    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0,
                           addKernelAddressSanitizerPasses);
  }

  if (LangOpts.Sanitize.has(SanitizerKind::Memory)) {
    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast,
                           addMemorySanitizerPass);
    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0,
                           addMemorySanitizerPass);
  }

  if (LangOpts.Sanitize.has(SanitizerKind::Thread)) {
    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast,
                           addThreadSanitizerPass);
    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0,
                           addThreadSanitizerPass);
  }

  if (LangOpts.Sanitize.has(SanitizerKind::DataFlow)) {
    PMBuilder.addExtension(PassManagerBuilder::EP_OptimizerLast,
                           addDataFlowSanitizerPass);
    PMBuilder.addExtension(PassManagerBuilder::EP_EnabledOnOptLevel0,
                           addDataFlowSanitizerPass);
  }

#endif // MS_ENABLE_INSTR - HLSL Change

  // Figure out TargetLibraryInfo.
  Triple TargetTriple(TheModule->getTargetTriple());
  PMBuilder.LibraryInfo = createTLII(TargetTriple, CodeGenOpts);

  switch (Inlining) {
  case CodeGenOptions::NoInlining: break;
  case CodeGenOptions::NormalInlining: {
    PMBuilder.HLSLEarlyInlining = false; // HLSL Change
    PMBuilder.Inliner =
        createFunctionInliningPass(OptLevel, CodeGenOpts.OptimizeSize);
    break;
  }
  case CodeGenOptions::OnlyAlwaysInlining:
    // Respect always_inline.
    if (OptLevel == 0)
      // Do not insert lifetime intrinsics at -O0.
      PMBuilder.Inliner = createAlwaysInlinerPass(false);
    else
      PMBuilder.Inliner = createAlwaysInlinerPass(PMBuilder.HLSLEnableLifetimeMarkers); // HLSL Change
    break;
  }

  // Set up the per-function pass manager.
  legacy::FunctionPassManager *FPM = getPerFunctionPasses();
  if (CodeGenOpts.VerifyModule)
    FPM->add(createVerifierPass());
  PMBuilder.populateFunctionPassManager(*FPM);

  // Set up the per-module pass manager.
  legacy::PassManager *MPM = getPerModulePasses();
  if (!CodeGenOpts.RewriteMapFiles.empty())
    addSymbolRewriterPass(CodeGenOpts, MPM);

#ifdef MS_ENABLE_INSTR // HLSL Change
  if (!CodeGenOpts.DisableGCov &&
      (CodeGenOpts.EmitGcovArcs || CodeGenOpts.EmitGcovNotes)) {
    // Not using 'GCOVOptions::getDefault' allows us to avoid exiting if
    // LLVM's -default-gcov-version flag is set to something invalid.
    GCOVOptions Options;
    Options.EmitNotes = CodeGenOpts.EmitGcovNotes;
    Options.EmitData = CodeGenOpts.EmitGcovArcs;
    memcpy(Options.Version, CodeGenOpts.CoverageVersion, 4);
    Options.UseCfgChecksum = CodeGenOpts.CoverageExtraChecksum;
    Options.NoRedZone = CodeGenOpts.DisableRedZone;
    Options.FunctionNamesInData =
        !CodeGenOpts.CoverageNoFunctionNamesInData;
    Options.ExitBlockBeforeBody = CodeGenOpts.CoverageExitBlockBeforeBody;
    MPM->add(createGCOVProfilerPass(Options));
    if (CodeGenOpts.getDebugInfo() == CodeGenOptions::NoDebugInfo)
      MPM->add(createStripSymbolsPass(true));
  }

  if (CodeGenOpts.ProfileInstrGenerate) {
    InstrProfOptions Options;
    Options.NoRedZone = CodeGenOpts.DisableRedZone;
    Options.InstrProfileOutput = CodeGenOpts.InstrProfileOutput;
    MPM->add(createInstrProfilingPass(Options));
  }
#endif

  PMBuilder.populateModulePassManager(*MPM);
}

TargetMachine *EmitAssemblyHelper::CreateTargetMachine(bool MustCreateTM) {
  // Create the TargetMachine for generating code.
  std::string Error;
  std::string Triple = TheModule->getTargetTriple();
  const llvm::Target *TheTarget = TargetRegistry::lookupTarget(Triple, Error);
  if (!TheTarget) {
    if (MustCreateTM)
      Diags.Report(diag::err_fe_unable_to_create_target) << Error;
    return nullptr;
  }

  unsigned CodeModel =
    llvm::StringSwitch<unsigned>(CodeGenOpts.CodeModel)
      .Case("small", llvm::CodeModel::Small)
      .Case("kernel", llvm::CodeModel::Kernel)
      .Case("medium", llvm::CodeModel::Medium)
      .Case("large", llvm::CodeModel::Large)
      .Case("default", llvm::CodeModel::Default)
      .Default(~0u);
  assert(CodeModel != ~0u && "invalid code model!");
  llvm::CodeModel::Model CM = static_cast<llvm::CodeModel::Model>(CodeModel);

#if 0 // HLSL Change - no global state mutation on per-instance work
  SmallVector<const char *, 16> BackendArgs;
  BackendArgs.push_back("clang"); // Fake program name.
  if (!CodeGenOpts.DebugPass.empty()) {
    BackendArgs.push_back("-debug-pass");
    BackendArgs.push_back(CodeGenOpts.DebugPass.c_str());
  }
  if (!CodeGenOpts.LimitFloatPrecision.empty()) {
    BackendArgs.push_back("-limit-float-precision");
    BackendArgs.push_back(CodeGenOpts.LimitFloatPrecision.c_str());
  }
  for (unsigned i = 0, e = CodeGenOpts.BackendOptions.size(); i != e; ++i)
    BackendArgs.push_back(CodeGenOpts.BackendOptions[i].c_str());
  BackendArgs.push_back(nullptr);
  llvm::cl::ParseCommandLineOptions(BackendArgs.size() - 1,
                                    BackendArgs.data());
#endif // HLSL Change

  std::string FeaturesStr;
  if (!TargetOpts.Features.empty()) {
#if 0 // HLSL Change
    SubtargetFeatures Features;
    for (const std::string &Feature : TargetOpts.Features)
      Features.AddFeature(Feature);
    FeaturesStr = Features.getString();
#endif // HLSL Change
  }

  llvm::Reloc::Model RM = llvm::Reloc::Default;
  if (CodeGenOpts.RelocationModel == "static") {
    RM = llvm::Reloc::Static;
  } else if (CodeGenOpts.RelocationModel == "pic") {
    RM = llvm::Reloc::PIC_;
  } else {
    assert(CodeGenOpts.RelocationModel == "dynamic-no-pic" &&
           "Invalid PIC model!");
    RM = llvm::Reloc::DynamicNoPIC;
  }

  CodeGenOpt::Level OptLevel = CodeGenOpt::Default;
  switch (CodeGenOpts.OptimizationLevel) {
  default: break;
  case 0: OptLevel = CodeGenOpt::None; break;
  case 3: OptLevel = CodeGenOpt::Aggressive; break;
  }

  llvm::TargetOptions Options;

  if (!TargetOpts.Reciprocals.empty())
    Options.Reciprocals = TargetRecip(TargetOpts.Reciprocals);

  Options.ThreadModel =
    llvm::StringSwitch<llvm::ThreadModel::Model>(CodeGenOpts.ThreadModel)
      .Case("posix", llvm::ThreadModel::POSIX)
      .Case("single", llvm::ThreadModel::Single);

  if (CodeGenOpts.DisableIntegratedAS)
    Options.DisableIntegratedAS = true;

  if (CodeGenOpts.CompressDebugSections)
    Options.CompressDebugSections = true;

  if (CodeGenOpts.UseInitArray)
    Options.UseInitArray = true;

  // Set float ABI type.
  if (CodeGenOpts.FloatABI == "soft" || CodeGenOpts.FloatABI == "softfp")
    Options.FloatABIType = llvm::FloatABI::Soft;
  else if (CodeGenOpts.FloatABI == "hard")
    Options.FloatABIType = llvm::FloatABI::Hard;
  else {
    assert(CodeGenOpts.FloatABI.empty() && "Invalid float abi!");
    Options.FloatABIType = llvm::FloatABI::Default;
  }

  // Set FP fusion mode.
  switch (CodeGenOpts.getFPContractMode()) {
  case CodeGenOptions::FPC_Off:
    Options.AllowFPOpFusion = llvm::FPOpFusion::Strict;
    break;
  case CodeGenOptions::FPC_On:
    Options.AllowFPOpFusion = llvm::FPOpFusion::Standard;
    break;
  case CodeGenOptions::FPC_Fast:
    Options.AllowFPOpFusion = llvm::FPOpFusion::Fast;
    break;
  }

  Options.LessPreciseFPMADOption = CodeGenOpts.LessPreciseFPMAD;
  Options.NoInfsFPMath = CodeGenOpts.NoInfsFPMath;
  Options.NoNaNsFPMath = CodeGenOpts.NoNaNsFPMath;
  Options.NoZerosInBSS = CodeGenOpts.NoZeroInitializedInBSS;
  Options.UnsafeFPMath = CodeGenOpts.UnsafeFPMath;
  Options.StackAlignmentOverride = CodeGenOpts.StackAlignment;
  Options.PositionIndependentExecutable = LangOpts.PIELevel != 0;
  Options.FunctionSections = CodeGenOpts.FunctionSections;
  Options.DataSections = CodeGenOpts.DataSections;
  Options.UniqueSectionNames = CodeGenOpts.UniqueSectionNames;

  Options.MCOptions.MCRelaxAll = CodeGenOpts.RelaxAll;
  Options.MCOptions.MCSaveTempLabels = CodeGenOpts.SaveTempLabels;
  Options.MCOptions.MCUseDwarfDirectory = !CodeGenOpts.NoDwarfDirectoryAsm;
  Options.MCOptions.MCNoExecStack = CodeGenOpts.NoExecStack;
  Options.MCOptions.MCFatalWarnings = CodeGenOpts.FatalWarnings;
  Options.MCOptions.AsmVerbose = CodeGenOpts.AsmVerbose;
  Options.MCOptions.ABIName = TargetOpts.ABI;

  TargetMachine *TM = TheTarget->createTargetMachine(Triple, TargetOpts.CPU,
                                                     FeaturesStr, Options,
                                                     RM, CM, OptLevel);

  return TM;
}

bool EmitAssemblyHelper::AddEmitPasses(BackendAction Action,
                                       raw_pwrite_stream &OS) {

  // Create the code generator passes.
  legacy::PassManager *PM = getCodeGenPasses();

  // Add LibraryInfo.
  llvm::Triple TargetTriple(TheModule->getTargetTriple());
  std::unique_ptr<TargetLibraryInfoImpl> TLII(
      createTLII(TargetTriple, CodeGenOpts));
  PM->add(new TargetLibraryInfoWrapperPass(*TLII));

  // Normal mode, emit a .s or .o file by running the code generator. Note,
  // this also adds codegenerator level optimization passes.
  TargetMachine::CodeGenFileType CGFT = TargetMachine::CGFT_AssemblyFile;
  if (Action == Backend_EmitObj)
    CGFT = TargetMachine::CGFT_ObjectFile;
  else if (Action == Backend_EmitMCNull)
    CGFT = TargetMachine::CGFT_Null;
  else
    assert(Action == Backend_EmitAssembly && "Invalid action!");

  // Add ObjC ARC final-cleanup optimizations. This is done as part of the
  // "codegen" passes so that it isn't run multiple times when there is
  // inlining happening.
#ifdef MS_ENABLE_OBJCARC // HLSL Change
  if (CodeGenOpts.OptimizationLevel > 0)
    PM->add(createObjCARCContractPass());
#endif // HLSL Change

  if (TM->addPassesToEmitFile(*PM, OS, CGFT,
                              /*DisableVerify=*/!CodeGenOpts.VerifyModule)) {
    Diags.Report(diag::err_fe_unable_to_interface_with_target);
    return false;
  }

  return true;
}

void EmitAssemblyHelper::EmitAssembly(BackendAction Action,
                                      raw_pwrite_stream *OS) {
  TimeRegion Region(llvm::TimePassesIsEnabled ? &CodeGenerationTime : nullptr);

  bool UsesCodeGen = (Action != Backend_EmitNothing &&
                      Action != Backend_EmitBC &&
                      Action != Backend_EmitPasses &&
                      Action != Backend_EmitLL);
  if (!TM)
    TM.reset(CreateTargetMachine(UsesCodeGen));

  if (UsesCodeGen && !TM)
    return;
  if (TM)
    TheModule->setDataLayout(*TM->getDataLayout());
  CreatePasses();

  switch (Action) {
  case Backend_EmitNothing:
  case Backend_EmitPasses: // HLSL Change
    break;

  case Backend_EmitBC:
    getPerModulePasses()->add(
        createBitcodeWriterPass(*OS, CodeGenOpts.EmitLLVMUseLists));
    break;

  case Backend_EmitLL:
    getPerModulePasses()->add(
        createPrintModulePass(*OS, "", CodeGenOpts.EmitLLVMUseLists));
    break;

  default:
    if (!AddEmitPasses(Action, *OS))
      return;
  }

  // Before executing passes, print the final values of the LLVM options.
  cl::PrintOptionValues();

  // HLSL Change Starts
  if (Action == Backend_EmitPasses) {
    if (PerFunctionPasses) {
      *OS << "# Per-function passes\n"
             "-opt-fn-passes\n";
      *OS << PerFunctionPassesConfigOS.str();
    }
    if (PerModulePasses) {
      *OS << "# Per-module passes\n"
             "-opt-mod-passes\n";
      *OS << PerModulePassesConfigOS.str();
    }
    if (CodeGenPasses) {
      *OS << "# Code generation passes\n"
             "-opt-mod-passes\n";
      *OS << CodeGenPassesConfigOS.str();
    }
    return;
  }
  // HLSL Change Ends

  // Run passes. For now we do all passes at once, but eventually we
  // would like to have the option of streaming code generation.

  if (PerFunctionPasses) {
    PrettyStackTraceString CrashInfo("Per-function optimization");

    PerFunctionPasses->doInitialization();
    for (Function &F : *TheModule)
      if (!F.isDeclaration())
        PerFunctionPasses->run(F);
    PerFunctionPasses->doFinalization();
  }

  if (PerModulePasses) {
    PrettyStackTraceString CrashInfo("Per-module optimization passes");
    PerModulePasses->run(*TheModule);
  }

  if (CodeGenPasses) {
    PrettyStackTraceString CrashInfo("Code generation");
    CodeGenPasses->run(*TheModule);
  }
}

void clang::EmitBackendOutput(DiagnosticsEngine &Diags,
                              const CodeGenOptions &CGOpts,
                              const clang::TargetOptions &TOpts,
                              const LangOptions &LOpts, StringRef TDesc,
                              Module *M, BackendAction Action,
                              raw_pwrite_stream *OS) {
  EmitAssemblyHelper AsmHelper(Diags, CGOpts, TOpts, LOpts, M);

  // HLSL Change - Support hierarchial time tracing.
  TimeTraceScope TimeScope("Backend", StringRef(""));

  try { // HLSL Change Starts
    // Catch any fatal errors during optimization passes here
    // so that future passes can be skipped.
    AsmHelper.EmitAssembly(Action, OS);
  } catch (const ::hlsl::Exception &hlslException) {
    Diags.Report(Diags.getCustomDiagID(DiagnosticsEngine::Error, "%0\n"))
        << StringRef(hlslException.what());
#if defined(DXC_CODEGEN_EXCEPTIONS_TRAP)
    // llvm::errs() doesn't work in release builds on Linux.
    // Use C-style fprintf because it works everywhere.
    fprintf(stderr, "internal codegen error: %s\n", hlslException.what());
    fflush(stderr);
    LLVM_BUILTIN_TRAP;
#endif
  } // HLSL Change Ends

  // If an optional clang TargetInfo description string was passed in, use it to
  // verify the LLVM TargetMachine's DataLayout.
  if (AsmHelper.TM && !TDesc.empty()) {
    std::string DLDesc =
        AsmHelper.TM->getDataLayout()->getStringRepresentation();
    if (DLDesc != TDesc) {
      unsigned DiagID = Diags.getCustomDiagID(
          DiagnosticsEngine::Error, "backend data layout '%0' does not match "
                                    "expected target description '%1'");
      Diags.Report(DiagID) << DLDesc << TDesc;
    }
  }
}
