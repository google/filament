//===--- AnalysisConsumer.cpp - ASTConsumer for running Analyses ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// "Meta" ASTConsumer for running different source analyses.
//
//===----------------------------------------------------------------------===//

#include "clang/StaticAnalyzer/Frontend/AnalysisConsumer.h"
#include "ModelInjector.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/DataRecursiveASTVisitor.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/DeclObjC.h"
#include "clang/AST/ParentMap.h"
#include "clang/Analysis/Analyses/LiveVariables.h"
#include "clang/Analysis/CFG.h"
#include "clang/Analysis/CallGraph.h"
#include "clang/Analysis/CodeInjector.h"
#include "clang/Basic/FileManager.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/StaticAnalyzer/Checkers/LocalCheckers.h"
#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporter.h"
#include "clang/StaticAnalyzer/Core/BugReporter/PathDiagnostic.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathDiagnosticConsumers.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/AnalysisManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ExprEngine.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistration.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <queue>

using namespace clang;
using namespace ento;
using llvm::SmallPtrSet;

#define DEBUG_TYPE "AnalysisConsumer"

static std::unique_ptr<ExplodedNode::Auditor> CreateUbiViz();

STATISTIC(NumFunctionTopLevel, "The # of functions at top level.");
STATISTIC(NumFunctionsAnalyzed,
                      "The # of functions and blocks analyzed (as top level "
                      "with inlining turned on).");
STATISTIC(NumBlocksInAnalyzedFunctions,
                      "The # of basic blocks in the analyzed functions.");
STATISTIC(PercentReachableBlocks, "The % of reachable basic blocks.");
STATISTIC(MaxCFGSize, "The maximum number of basic blocks in a function.");

//===----------------------------------------------------------------------===//
// Special PathDiagnosticConsumers.
//===----------------------------------------------------------------------===//

void ento::createPlistHTMLDiagnosticConsumer(AnalyzerOptions &AnalyzerOpts,
                                             PathDiagnosticConsumers &C,
                                             const std::string &prefix,
                                             const Preprocessor &PP) {
  createHTMLDiagnosticConsumer(AnalyzerOpts, C,
                               llvm::sys::path::parent_path(prefix), PP);
  createPlistDiagnosticConsumer(AnalyzerOpts, C, prefix, PP);
}

void ento::createTextPathDiagnosticConsumer(AnalyzerOptions &AnalyzerOpts,
                                            PathDiagnosticConsumers &C,
                                            const std::string &Prefix,
                                            const clang::Preprocessor &PP) {
  llvm_unreachable("'text' consumer should be enabled on ClangDiags");
}

namespace {
class ClangDiagPathDiagConsumer : public PathDiagnosticConsumer {
  DiagnosticsEngine &Diag;
  bool IncludePath;
public:
  ClangDiagPathDiagConsumer(DiagnosticsEngine &Diag)
    : Diag(Diag), IncludePath(false) {}
  ~ClangDiagPathDiagConsumer() override {}
  StringRef getName() const override { return "ClangDiags"; }

  bool supportsLogicalOpControlFlow() const override { return true; }
  bool supportsCrossFileDiagnostics() const override { return true; }

  PathGenerationScheme getGenerationScheme() const override {
    return IncludePath ? Minimal : None;
  }

  void enablePaths() {
    IncludePath = true;
  }

  void FlushDiagnosticsImpl(std::vector<const PathDiagnostic *> &Diags,
                            FilesMade *filesMade) override {
    unsigned WarnID = Diag.getCustomDiagID(DiagnosticsEngine::Warning, "%0");
    unsigned NoteID = Diag.getCustomDiagID(DiagnosticsEngine::Note, "%0");

    for (std::vector<const PathDiagnostic*>::iterator I = Diags.begin(),
         E = Diags.end(); I != E; ++I) {
      const PathDiagnostic *PD = *I;
      SourceLocation WarnLoc = PD->getLocation().asLocation();
      Diag.Report(WarnLoc, WarnID) << PD->getShortDescription()
                                   << PD->path.back()->getRanges();

      if (!IncludePath)
        continue;

      PathPieces FlatPath = PD->path.flatten(/*ShouldFlattenMacros=*/true);
      for (PathPieces::const_iterator PI = FlatPath.begin(),
                                      PE = FlatPath.end();
           PI != PE; ++PI) {
        SourceLocation NoteLoc = (*PI)->getLocation().asLocation();
        Diag.Report(NoteLoc, NoteID) << (*PI)->getString()
                                     << (*PI)->getRanges();
      }
    }
  }
};
} // end anonymous namespace

//===----------------------------------------------------------------------===//
// AnalysisConsumer declaration.
//===----------------------------------------------------------------------===//

namespace {

class AnalysisConsumer : public AnalysisASTConsumer,
                         public DataRecursiveASTVisitor<AnalysisConsumer> {
  enum {
    AM_None = 0,
    AM_Syntax = 0x1,
    AM_Path = 0x2
  };
  typedef unsigned AnalysisMode;

  /// Mode of the analyzes while recursively visiting Decls.
  AnalysisMode RecVisitorMode;
  /// Bug Reporter to use while recursively visiting Decls.
  BugReporter *RecVisitorBR;

public:
  ASTContext *Ctx;
  const Preprocessor &PP;
  const std::string OutDir;
  AnalyzerOptionsRef Opts;
  ArrayRef<std::string> Plugins;
  CodeInjector *Injector;

  /// \brief Stores the declarations from the local translation unit.
  /// Note, we pre-compute the local declarations at parse time as an
  /// optimization to make sure we do not deserialize everything from disk.
  /// The local declaration to all declarations ratio might be very small when
  /// working with a PCH file.
  SetOfDecls LocalTUDecls;
                           
  // Set of PathDiagnosticConsumers.  Owned by AnalysisManager.
  PathDiagnosticConsumers PathConsumers;

  StoreManagerCreator CreateStoreMgr;
  ConstraintManagerCreator CreateConstraintMgr;

  std::unique_ptr<CheckerManager> checkerMgr;
  std::unique_ptr<AnalysisManager> Mgr;

  /// Time the analyzes time of each translation unit.
  static llvm::Timer* TUTotalTimer;

  /// The information about analyzed functions shared throughout the
  /// translation unit.
  FunctionSummariesTy FunctionSummaries;

  AnalysisConsumer(const Preprocessor& pp,
                   const std::string& outdir,
                   AnalyzerOptionsRef opts,
                   ArrayRef<std::string> plugins,
                   CodeInjector *injector)
    : RecVisitorMode(0), RecVisitorBR(nullptr), Ctx(nullptr), PP(pp),
      OutDir(outdir), Opts(opts), Plugins(plugins), Injector(injector) {
    DigestAnalyzerOptions();
    if (Opts->PrintStats) {
      llvm::EnableStatistics();
      TUTotalTimer = new llvm::Timer("Analyzer Total Time");
    }
  }

  ~AnalysisConsumer() override {
    if (Opts->PrintStats)
      delete TUTotalTimer;
  }

  void DigestAnalyzerOptions() {
    if (Opts->AnalysisDiagOpt != PD_NONE) {
      // Create the PathDiagnosticConsumer.
      ClangDiagPathDiagConsumer *clangDiags =
          new ClangDiagPathDiagConsumer(PP.getDiagnostics());
      PathConsumers.push_back(clangDiags);

      if (Opts->AnalysisDiagOpt == PD_TEXT) {
        clangDiags->enablePaths();

      } else if (!OutDir.empty()) {
        switch (Opts->AnalysisDiagOpt) {
        default:
#define ANALYSIS_DIAGNOSTICS(NAME, CMDFLAG, DESC, CREATEFN)                    \
  case PD_##NAME:                                                              \
    CREATEFN(*Opts.get(), PathConsumers, OutDir, PP);                       \
    break;
#include "clang/StaticAnalyzer/Core/Analyses.def"
        }
      }
    }

    // Create the analyzer component creators.
    switch (Opts->AnalysisStoreOpt) {
    default:
      llvm_unreachable("Unknown store manager.");
#define ANALYSIS_STORE(NAME, CMDFLAG, DESC, CREATEFN)           \
      case NAME##Model: CreateStoreMgr = CREATEFN; break;
#include "clang/StaticAnalyzer/Core/Analyses.def"
    }

    switch (Opts->AnalysisConstraintsOpt) {
    default:
      llvm_unreachable("Unknown constraint manager.");
#define ANALYSIS_CONSTRAINTS(NAME, CMDFLAG, DESC, CREATEFN)     \
      case NAME##Model: CreateConstraintMgr = CREATEFN; break;
#include "clang/StaticAnalyzer/Core/Analyses.def"
    }
  }

  void DisplayFunction(const Decl *D, AnalysisMode Mode,
                       ExprEngine::InliningModes IMode) {
    if (!Opts->AnalyzerDisplayProgress)
      return;

    SourceManager &SM = Mgr->getASTContext().getSourceManager();
    PresumedLoc Loc = SM.getPresumedLoc(D->getLocation());
    if (Loc.isValid()) {
      llvm::errs() << "ANALYZE";

      if (Mode == AM_Syntax)
        llvm::errs() << " (Syntax)";
      else if (Mode == AM_Path) {
        llvm::errs() << " (Path, ";
        switch (IMode) {
          case ExprEngine::Inline_Minimal:
            llvm::errs() << " Inline_Minimal";
            break;
          case ExprEngine::Inline_Regular:
            llvm::errs() << " Inline_Regular";
            break;
        }
        llvm::errs() << ")";
      }
      else
        assert(Mode == (AM_Syntax | AM_Path) && "Unexpected mode!");

      llvm::errs() << ": " << Loc.getFilename();
      if (isa<FunctionDecl>(D) || isa<ObjCMethodDecl>(D)) {
        const NamedDecl *ND = cast<NamedDecl>(D);
        llvm::errs() << ' ' << *ND << '\n';
      }
      else if (isa<BlockDecl>(D)) {
        llvm::errs() << ' ' << "block(line:" << Loc.getLine() << ",col:"
                     << Loc.getColumn() << '\n';
      }
      else if (const ObjCMethodDecl *MD = dyn_cast<ObjCMethodDecl>(D)) {
        Selector S = MD->getSelector();
        llvm::errs() << ' ' << S.getAsString();
      }
    }
  }

  void Initialize(ASTContext &Context) override {
    Ctx = &Context;
    checkerMgr = createCheckerManager(*Opts, PP.getLangOpts(), Plugins,
                                      PP.getDiagnostics());

    Mgr = llvm::make_unique<AnalysisManager>(
        *Ctx, PP.getDiagnostics(), PP.getLangOpts(), PathConsumers,
        CreateStoreMgr, CreateConstraintMgr, checkerMgr.get(), *Opts, Injector);
  }

  /// \brief Store the top level decls in the set to be processed later on.
  /// (Doing this pre-processing avoids deserialization of data from PCH.)
  bool HandleTopLevelDecl(DeclGroupRef D) override;
  void HandleTopLevelDeclInObjCContainer(DeclGroupRef D) override;

  void HandleTranslationUnit(ASTContext &C) override;

  /// \brief Determine which inlining mode should be used when this function is
  /// analyzed. This allows to redefine the default inlining policies when
  /// analyzing a given function.
  ExprEngine::InliningModes
    getInliningModeForFunction(const Decl *D, const SetOfConstDecls &Visited);

  /// \brief Build the call graph for all the top level decls of this TU and
  /// use it to define the order in which the functions should be visited.
  void HandleDeclsCallGraph(const unsigned LocalTUDeclsSize);

  /// \brief Run analyzes(syntax or path sensitive) on the given function.
  /// \param Mode - determines if we are requesting syntax only or path
  /// sensitive only analysis.
  /// \param VisitedCallees - The output parameter, which is populated with the
  /// set of functions which should be considered analyzed after analyzing the
  /// given root function.
  void HandleCode(Decl *D, AnalysisMode Mode,
                  ExprEngine::InliningModes IMode = ExprEngine::Inline_Minimal,
                  SetOfConstDecls *VisitedCallees = nullptr);

  void RunPathSensitiveChecks(Decl *D,
                              ExprEngine::InliningModes IMode,
                              SetOfConstDecls *VisitedCallees);
  void ActionExprEngine(Decl *D, bool ObjCGCEnabled,
                        ExprEngine::InliningModes IMode,
                        SetOfConstDecls *VisitedCallees);

  /// Visitors for the RecursiveASTVisitor.
  bool shouldWalkTypesOfTypeLocs() const { return false; }

  /// Handle callbacks for arbitrary Decls.
  bool VisitDecl(Decl *D) {
    AnalysisMode Mode = getModeForDecl(D, RecVisitorMode);
    if (Mode & AM_Syntax)
      checkerMgr->runCheckersOnASTDecl(D, *Mgr, *RecVisitorBR);
    return true;
  }

  bool VisitFunctionDecl(FunctionDecl *FD) {
    IdentifierInfo *II = FD->getIdentifier();
    if (II && II->getName().startswith("__inline"))
      return true;

    // We skip function template definitions, as their semantics is
    // only determined when they are instantiated.
    if (FD->isThisDeclarationADefinition() &&
        !FD->isDependentContext()) {
      assert(RecVisitorMode == AM_Syntax || Mgr->shouldInlineCall() == false);
      HandleCode(FD, RecVisitorMode);
    }
    return true;
  }

  bool VisitObjCMethodDecl(ObjCMethodDecl *MD) {
    if (MD->isThisDeclarationADefinition()) {
      assert(RecVisitorMode == AM_Syntax || Mgr->shouldInlineCall() == false);
      HandleCode(MD, RecVisitorMode);
    }
    return true;
  }
  
  bool VisitBlockDecl(BlockDecl *BD) {
    if (BD->hasBody()) {
      assert(RecVisitorMode == AM_Syntax || Mgr->shouldInlineCall() == false);
      HandleCode(BD, RecVisitorMode);
    }
    return true;
  }

  void AddDiagnosticConsumer(PathDiagnosticConsumer *Consumer) override {
    PathConsumers.push_back(Consumer);
  }

private:
  void storeTopLevelDecls(DeclGroupRef DG);

  /// \brief Check if we should skip (not analyze) the given function.
  AnalysisMode getModeForDecl(Decl *D, AnalysisMode Mode);

};
} // end anonymous namespace


//===----------------------------------------------------------------------===//
// AnalysisConsumer implementation.
//===----------------------------------------------------------------------===//
llvm::Timer* AnalysisConsumer::TUTotalTimer = nullptr;

bool AnalysisConsumer::HandleTopLevelDecl(DeclGroupRef DG) {
  storeTopLevelDecls(DG);
  return true;
}

void AnalysisConsumer::HandleTopLevelDeclInObjCContainer(DeclGroupRef DG) {
  storeTopLevelDecls(DG);
}

void AnalysisConsumer::storeTopLevelDecls(DeclGroupRef DG) {
  for (DeclGroupRef::iterator I = DG.begin(), E = DG.end(); I != E; ++I) {

    // Skip ObjCMethodDecl, wait for the objc container to avoid
    // analyzing twice.
    if (isa<ObjCMethodDecl>(*I))
      continue;

    LocalTUDecls.push_back(*I);
  }
}

static bool shouldSkipFunction(const Decl *D,
                               const SetOfConstDecls &Visited,
                               const SetOfConstDecls &VisitedAsTopLevel) {
  if (VisitedAsTopLevel.count(D))
    return true;

  // We want to re-analyse the functions as top level in the following cases:
  // - The 'init' methods should be reanalyzed because
  //   ObjCNonNilReturnValueChecker assumes that '[super init]' never returns
  //   'nil' and unless we analyze the 'init' functions as top level, we will
  //   not catch errors within defensive code.
  // - We want to reanalyze all ObjC methods as top level to report Retain
  //   Count naming convention errors more aggressively.
  if (isa<ObjCMethodDecl>(D))
    return false;

  // Otherwise, if we visited the function before, do not reanalyze it.
  return Visited.count(D);
}

ExprEngine::InliningModes
AnalysisConsumer::getInliningModeForFunction(const Decl *D,
                                             const SetOfConstDecls &Visited) {
  // We want to reanalyze all ObjC methods as top level to report Retain
  // Count naming convention errors more aggressively. But we should tune down
  // inlining when reanalyzing an already inlined function.
  if (Visited.count(D)) {
    assert(isa<ObjCMethodDecl>(D) &&
           "We are only reanalyzing ObjCMethods.");
    const ObjCMethodDecl *ObjCM = cast<ObjCMethodDecl>(D);
    if (ObjCM->getMethodFamily() != OMF_init)
      return ExprEngine::Inline_Minimal;
  }

  return ExprEngine::Inline_Regular;
}

void AnalysisConsumer::HandleDeclsCallGraph(const unsigned LocalTUDeclsSize) {
  // Build the Call Graph by adding all the top level declarations to the graph.
  // Note: CallGraph can trigger deserialization of more items from a pch
  // (though HandleInterestingDecl); triggering additions to LocalTUDecls.
  // We rely on random access to add the initially processed Decls to CG.
  CallGraph CG;
  for (unsigned i = 0 ; i < LocalTUDeclsSize ; ++i) {
    CG.addToCallGraph(LocalTUDecls[i]);
  }

  // Walk over all of the call graph nodes in topological order, so that we
  // analyze parents before the children. Skip the functions inlined into
  // the previously processed functions. Use external Visited set to identify
  // inlined functions. The topological order allows the "do not reanalyze
  // previously inlined function" performance heuristic to be triggered more
  // often.
  SetOfConstDecls Visited;
  SetOfConstDecls VisitedAsTopLevel;
  llvm::ReversePostOrderTraversal<clang::CallGraph*> RPOT(&CG);
  for (llvm::ReversePostOrderTraversal<clang::CallGraph*>::rpo_iterator
         I = RPOT.begin(), E = RPOT.end(); I != E; ++I) {
    NumFunctionTopLevel++;

    CallGraphNode *N = *I;
    Decl *D = N->getDecl();
    
    // Skip the abstract root node.
    if (!D)
      continue;

    // Skip the functions which have been processed already or previously
    // inlined.
    if (shouldSkipFunction(D, Visited, VisitedAsTopLevel))
      continue;

    // Analyze the function.
    SetOfConstDecls VisitedCallees;

    HandleCode(D, AM_Path, getInliningModeForFunction(D, Visited),
               (Mgr->options.InliningMode == All ? nullptr : &VisitedCallees));

    // Add the visited callees to the global visited set.
    for (SetOfConstDecls::iterator I = VisitedCallees.begin(),
                                   E = VisitedCallees.end(); I != E; ++I) {
        Visited.insert(*I);
    }
    VisitedAsTopLevel.insert(D);
  }
}

void AnalysisConsumer::HandleTranslationUnit(ASTContext &C) {
  // Don't run the actions if an error has occurred with parsing the file.
  DiagnosticsEngine &Diags = PP.getDiagnostics();
  if (Diags.hasErrorOccurred() || Diags.hasFatalErrorOccurred())
    return;

  // Don't analyze if the user explicitly asked for no checks to be performed
  // on this file.
  if (Opts->DisableAllChecks)
    return;

  {
    if (TUTotalTimer) TUTotalTimer->startTimer();

    // Introduce a scope to destroy BR before Mgr.
    BugReporter BR(*Mgr);
    TranslationUnitDecl *TU = C.getTranslationUnitDecl();
    checkerMgr->runCheckersOnASTDecl(TU, *Mgr, BR);

    // Run the AST-only checks using the order in which functions are defined.
    // If inlining is not turned on, use the simplest function order for path
    // sensitive analyzes as well.
    RecVisitorMode = AM_Syntax;
    if (!Mgr->shouldInlineCall())
      RecVisitorMode |= AM_Path;
    RecVisitorBR = &BR;

    // Process all the top level declarations.
    //
    // Note: TraverseDecl may modify LocalTUDecls, but only by appending more
    // entries.  Thus we don't use an iterator, but rely on LocalTUDecls
    // random access.  By doing so, we automatically compensate for iterators
    // possibly being invalidated, although this is a bit slower.
    const unsigned LocalTUDeclsSize = LocalTUDecls.size();
    for (unsigned i = 0 ; i < LocalTUDeclsSize ; ++i) {
      TraverseDecl(LocalTUDecls[i]);
    }

    if (Mgr->shouldInlineCall())
      HandleDeclsCallGraph(LocalTUDeclsSize);

    // After all decls handled, run checkers on the entire TranslationUnit.
    checkerMgr->runCheckersOnEndOfTranslationUnit(TU, *Mgr, BR);

    RecVisitorBR = nullptr;
  }

  // Explicitly destroy the PathDiagnosticConsumer.  This will flush its output.
  // FIXME: This should be replaced with something that doesn't rely on
  // side-effects in PathDiagnosticConsumer's destructor. This is required when
  // used with option -disable-free.
  Mgr.reset();

  if (TUTotalTimer) TUTotalTimer->stopTimer();

  // Count how many basic blocks we have not covered.
  NumBlocksInAnalyzedFunctions = FunctionSummaries.getTotalNumBasicBlocks();
  if (NumBlocksInAnalyzedFunctions > 0)
    PercentReachableBlocks =
      (FunctionSummaries.getTotalNumVisitedBasicBlocks() * 100) /
        NumBlocksInAnalyzedFunctions;

}

static std::string getFunctionName(const Decl *D) {
  if (const ObjCMethodDecl *ID = dyn_cast<ObjCMethodDecl>(D)) {
    return ID->getSelector().getAsString();
  }
  if (const FunctionDecl *ND = dyn_cast<FunctionDecl>(D)) {
    IdentifierInfo *II = ND->getIdentifier();
    if (II)
      return II->getName();
  }
  return "";
}

AnalysisConsumer::AnalysisMode
AnalysisConsumer::getModeForDecl(Decl *D, AnalysisMode Mode) {
  if (!Opts->AnalyzeSpecificFunction.empty() &&
      getFunctionName(D) != Opts->AnalyzeSpecificFunction)
    return AM_None;

  // Unless -analyze-all is specified, treat decls differently depending on
  // where they came from:
  // - Main source file: run both path-sensitive and non-path-sensitive checks.
  // - Header files: run non-path-sensitive checks only.
  // - System headers: don't run any checks.
  SourceManager &SM = Ctx->getSourceManager();
  SourceLocation SL = D->hasBody() ? D->getBody()->getLocStart()
                                     : D->getLocation();
  SL = SM.getExpansionLoc(SL);

  if (!Opts->AnalyzeAll && !SM.isWrittenInMainFile(SL)) {
    if (SL.isInvalid() || SM.isInSystemHeader(SL))
      return AM_None;
    return Mode & ~AM_Path;
  }

  return Mode;
}

void AnalysisConsumer::HandleCode(Decl *D, AnalysisMode Mode,
                                  ExprEngine::InliningModes IMode,
                                  SetOfConstDecls *VisitedCallees) {
  if (!D->hasBody())
    return;
  Mode = getModeForDecl(D, Mode);
  if (Mode == AM_None)
    return;

  DisplayFunction(D, Mode, IMode);
  CFG *DeclCFG = Mgr->getCFG(D);
  if (DeclCFG) {
    unsigned CFGSize = DeclCFG->size();
    MaxCFGSize = MaxCFGSize < CFGSize ? CFGSize : MaxCFGSize;
  }

  // Clear the AnalysisManager of old AnalysisDeclContexts.
  Mgr->ClearContexts();
  BugReporter BR(*Mgr);

  if (Mode & AM_Syntax)
    checkerMgr->runCheckersOnASTBody(D, *Mgr, BR);
  if ((Mode & AM_Path) && checkerMgr->hasPathSensitiveCheckers()) {
    RunPathSensitiveChecks(D, IMode, VisitedCallees);
    if (IMode != ExprEngine::Inline_Minimal)
      NumFunctionsAnalyzed++;
  }
}

//===----------------------------------------------------------------------===//
// Path-sensitive checking.
//===----------------------------------------------------------------------===//

void AnalysisConsumer::ActionExprEngine(Decl *D, bool ObjCGCEnabled,
                                        ExprEngine::InliningModes IMode,
                                        SetOfConstDecls *VisitedCallees) {
  // Construct the analysis engine.  First check if the CFG is valid.
  // FIXME: Inter-procedural analysis will need to handle invalid CFGs.
  if (!Mgr->getCFG(D))
    return;

  // See if the LiveVariables analysis scales.
  if (!Mgr->getAnalysisDeclContext(D)->getAnalysis<RelaxedLiveVariables>())
    return;

  ExprEngine Eng(*Mgr, ObjCGCEnabled, VisitedCallees, &FunctionSummaries,IMode);

  // Set the graph auditor.
  std::unique_ptr<ExplodedNode::Auditor> Auditor;
  if (Mgr->options.visualizeExplodedGraphWithUbiGraph) {
    Auditor = CreateUbiViz();
    ExplodedNode::SetAuditor(Auditor.get());
  }

  // Execute the worklist algorithm.
  Eng.ExecuteWorkList(Mgr->getAnalysisDeclContextManager().getStackFrame(D),
                      Mgr->options.getMaxNodesPerTopLevelFunction());

  // Release the auditor (if any) so that it doesn't monitor the graph
  // created BugReporter.
  ExplodedNode::SetAuditor(nullptr);

  // Visualize the exploded graph.
  if (Mgr->options.visualizeExplodedGraphWithGraphViz)
    Eng.ViewGraph(Mgr->options.TrimGraph);

  // Display warnings.
  Eng.getBugReporter().FlushReports();
}

void AnalysisConsumer::RunPathSensitiveChecks(Decl *D,
                                              ExprEngine::InliningModes IMode,
                                              SetOfConstDecls *Visited) {

  switch (Mgr->getLangOpts().getGC()) {
  case LangOptions::NonGC:
    ActionExprEngine(D, false, IMode, Visited);
    break;
  
  case LangOptions::GCOnly:
    ActionExprEngine(D, true, IMode, Visited);
    break;
  
  case LangOptions::HybridGC:
    ActionExprEngine(D, false, IMode, Visited);
    ActionExprEngine(D, true, IMode, Visited);
    break;
  }
}

//===----------------------------------------------------------------------===//
// AnalysisConsumer creation.
//===----------------------------------------------------------------------===//

std::unique_ptr<AnalysisASTConsumer>
ento::CreateAnalysisConsumer(CompilerInstance &CI) {
  // Disable the effects of '-Werror' when using the AnalysisConsumer.
  CI.getPreprocessor().getDiagnostics().setWarningsAsErrors(false);

  AnalyzerOptionsRef analyzerOpts = CI.getAnalyzerOpts();
  bool hasModelPath = analyzerOpts->Config.count("model-path") > 0;

  return llvm::make_unique<AnalysisConsumer>(
      CI.getPreprocessor(), CI.getFrontendOpts().OutputFile, analyzerOpts,
      CI.getFrontendOpts().Plugins,
      hasModelPath ? new ModelInjector(CI) : nullptr);
}

//===----------------------------------------------------------------------===//
// Ubigraph Visualization.  FIXME: Move to separate file.
//===----------------------------------------------------------------------===//

namespace {

class UbigraphViz : public ExplodedNode::Auditor {
  std::unique_ptr<raw_ostream> Out;
  std::string Filename;
  unsigned Cntr;

  typedef llvm::DenseMap<void*,unsigned> VMap;
  VMap M;

public:
  UbigraphViz(std::unique_ptr<raw_ostream> Out, StringRef Filename);

  ~UbigraphViz() override;

  void AddEdge(ExplodedNode *Src, ExplodedNode *Dst) override;
};

} // end anonymous namespace

static std::unique_ptr<ExplodedNode::Auditor> CreateUbiViz() {
  SmallString<128> P;
  int FD;
  llvm::sys::fs::createTemporaryFile("llvm_ubi", "", FD, P);
  llvm::errs() << "Writing '" << P << "'.\n";

  auto Stream = llvm::make_unique<llvm::raw_fd_ostream>(FD, true);

  return llvm::make_unique<UbigraphViz>(std::move(Stream), P);
}

void UbigraphViz::AddEdge(ExplodedNode *Src, ExplodedNode *Dst) {

  assert (Src != Dst && "Self-edges are not allowed.");

  // Lookup the Src.  If it is a new node, it's a root.
  VMap::iterator SrcI= M.find(Src);
  unsigned SrcID;

  if (SrcI == M.end()) {
    M[Src] = SrcID = Cntr++;
    *Out << "('vertex', " << SrcID << ", ('color','#00ff00'))\n";
  }
  else
    SrcID = SrcI->second;

  // Lookup the Dst.
  VMap::iterator DstI= M.find(Dst);
  unsigned DstID;

  if (DstI == M.end()) {
    M[Dst] = DstID = Cntr++;
    *Out << "('vertex', " << DstID << ")\n";
  }
  else {
    // We have hit DstID before.  Change its style to reflect a cache hit.
    DstID = DstI->second;
    *Out << "('change_vertex_style', " << DstID << ", 1)\n";
  }

  // Add the edge.
  *Out << "('edge', " << SrcID << ", " << DstID
       << ", ('arrow','true'), ('oriented', 'true'))\n";
}

UbigraphViz::UbigraphViz(std::unique_ptr<raw_ostream> Out, StringRef Filename)
    : Out(std::move(Out)), Filename(Filename), Cntr(0) {

  *Out << "('vertex_style_attribute', 0, ('shape', 'icosahedron'))\n";
  *Out << "('vertex_style', 1, 0, ('shape', 'sphere'), ('color', '#ffcc66'),"
          " ('size', '1.5'))\n";
}

UbigraphViz::~UbigraphViz() {
  Out.reset();
  llvm::errs() << "Running 'ubiviz' program... ";
#ifdef MSFT_SUPPORTS_CHILD_PROCESSES
  std::string ErrMsg;
  std::string Ubiviz;
  if (auto Path = llvm::sys::findProgramByName("ubiviz"))
    Ubiviz = *Path;
  std::vector<const char*> args;
  args.push_back(Ubiviz.c_str());
  args.push_back(Filename.c_str());
  args.push_back(nullptr);

  if (llvm::sys::ExecuteAndWait(Ubiviz, &args[0], nullptr, nullptr, 0, 0,
                                &ErrMsg)) {
    llvm::errs() << "Error viewing graph: " << ErrMsg << "\n";
  }

  // Delete the file.
  llvm::sys::fs::remove(Filename);
#endif // MSFT_SUPPORTS_CHILD_PROCESSES
}
