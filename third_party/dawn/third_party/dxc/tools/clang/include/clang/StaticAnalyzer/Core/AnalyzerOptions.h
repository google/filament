//===--- AnalyzerOptions.h - Analysis Engine Options ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This header defines various options for the static analyzer that are set
// by the frontend and are consulted throughout the analyzer.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CORE_ANALYZEROPTIONS_H
#define LLVM_CLANG_STATICANALYZER_CORE_ANALYZEROPTIONS_H

#include "clang/Basic/LLVM.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/StringMap.h"
#include <string>
#include <vector>

namespace clang {
class ASTConsumer;
class DiagnosticsEngine;
class Preprocessor;
class LangOptions;

namespace ento {
class CheckerBase;
}

/// Analysis - Set of available source code analyses.
enum Analyses {
#define ANALYSIS(NAME, CMDFLAG, DESC, SCOPE) NAME,
#include "clang/StaticAnalyzer/Core/Analyses.def"
NumAnalyses
};

/// AnalysisStores - Set of available analysis store models.
enum AnalysisStores {
#define ANALYSIS_STORE(NAME, CMDFLAG, DESC, CREATFN) NAME##Model,
#include "clang/StaticAnalyzer/Core/Analyses.def"
NumStores
};

/// AnalysisConstraints - Set of available constraint models.
enum AnalysisConstraints {
#define ANALYSIS_CONSTRAINTS(NAME, CMDFLAG, DESC, CREATFN) NAME##Model,
#include "clang/StaticAnalyzer/Core/Analyses.def"
NumConstraints
};

/// AnalysisDiagClients - Set of available diagnostic clients for rendering
///  analysis results.
enum AnalysisDiagClients {
#define ANALYSIS_DIAGNOSTICS(NAME, CMDFLAG, DESC, CREATFN) PD_##NAME,
#include "clang/StaticAnalyzer/Core/Analyses.def"
PD_NONE,
NUM_ANALYSIS_DIAG_CLIENTS
};

/// AnalysisPurgeModes - Set of available strategies for dead symbol removal.
enum AnalysisPurgeMode {
#define ANALYSIS_PURGE(NAME, CMDFLAG, DESC) NAME,
#include "clang/StaticAnalyzer/Core/Analyses.def"
NumPurgeModes
};

/// AnalysisInlineFunctionSelection - Set of inlining function selection heuristics.
enum AnalysisInliningMode {
#define ANALYSIS_INLINING_MODE(NAME, CMDFLAG, DESC) NAME,
#include "clang/StaticAnalyzer/Core/Analyses.def"
NumInliningModes
};

/// \brief Describes the different kinds of C++ member functions which can be
/// considered for inlining by the analyzer.
///
/// These options are cumulative; enabling one kind of member function will
/// enable all kinds with lower enum values.
enum CXXInlineableMemberKind {
  // Uninitialized = 0,

  /// A dummy mode in which no C++ inlining is enabled.
  CIMK_None = 1,

  /// Refers to regular member function and operator calls.
  CIMK_MemberFunctions,

  /// Refers to constructors (implicit or explicit).
  ///
  /// Note that a constructor will not be inlined if the corresponding
  /// destructor is non-trivial.
  CIMK_Constructors,

  /// Refers to destructors (implicit or explicit).
  CIMK_Destructors
};

/// \brief Describes the different modes of inter-procedural analysis.
enum IPAKind {
  IPAK_NotSet = 0,

  /// Perform only intra-procedural analysis.
  IPAK_None = 1,

  /// Inline C functions and blocks when their definitions are available.
  IPAK_BasicInlining = 2,

  /// Inline callees(C, C++, ObjC) when their definitions are available.
  IPAK_Inlining = 3,

  /// Enable inlining of dynamically dispatched methods.
  IPAK_DynamicDispatch = 4,

  /// Enable inlining of dynamically dispatched methods, bifurcate paths when
  /// exact type info is unavailable.
  IPAK_DynamicDispatchBifurcate = 5
};

class AnalyzerOptions : public RefCountedBase<AnalyzerOptions> {
public:
  typedef llvm::StringMap<std::string> ConfigTable;

  /// \brief Pair of checker name and enable/disable.
  std::vector<std::pair<std::string, bool> > CheckersControlList;
  
  /// \brief A key-value table of use-specified configuration values.
  ConfigTable Config;
  AnalysisStores AnalysisStoreOpt;
  AnalysisConstraints AnalysisConstraintsOpt;
  AnalysisDiagClients AnalysisDiagOpt;
  AnalysisPurgeMode AnalysisPurgeOpt;
  
  std::string AnalyzeSpecificFunction;
  
  /// \brief The maximum number of times the analyzer visits a block.
  unsigned maxBlockVisitOnPath;
  
  
  /// \brief Disable all analyzer checks.
  ///
  /// This flag allows one to disable analyzer checks on the code processed by
  /// the given analysis consumer. Note, the code will get parsed and the
  /// command-line options will get checked.
  unsigned DisableAllChecks : 1;

  unsigned ShowCheckerHelp : 1;
  unsigned AnalyzeAll : 1;
  unsigned AnalyzerDisplayProgress : 1;
  unsigned AnalyzeNestedBlocks : 1;

  /// \brief The flag regulates if we should eagerly assume evaluations of
  /// conditionals, thus, bifurcating the path.
  ///
  /// This flag indicates how the engine should handle expressions such as: 'x =
  /// (y != 0)'.  When this flag is true then the subexpression 'y != 0' will be
  /// eagerly assumed to be true or false, thus evaluating it to the integers 0
  /// or 1 respectively.  The upside is that this can increase analysis
  /// precision until we have a better way to lazily evaluate such logic.  The
  /// downside is that it eagerly bifurcates paths.
  unsigned eagerlyAssumeBinOpBifurcation : 1;
  
  unsigned TrimGraph : 1;
  unsigned visualizeExplodedGraphWithGraphViz : 1;
  unsigned visualizeExplodedGraphWithUbiGraph : 1;
  unsigned UnoptimizedCFG : 1;
  unsigned PrintStats : 1;
  
  /// \brief Do not re-analyze paths leading to exhausted nodes with a different
  /// strategy. We get better code coverage when retry is enabled.
  unsigned NoRetryExhausted : 1;
  
  /// \brief The inlining stack depth limit.
  unsigned InlineMaxStackDepth;
  
  /// \brief The mode of function selection used during inlining.
  AnalysisInliningMode InliningMode;

private:
  /// \brief Describes the kinds for high-level analyzer mode.
  enum UserModeKind {
    UMK_NotSet = 0,
    /// Perform shallow but fast analyzes.
    UMK_Shallow = 1,
    /// Perform deep analyzes.
    UMK_Deep = 2
  };

  /// Controls the high-level analyzer mode, which influences the default 
  /// settings for some of the lower-level config options (such as IPAMode).
  /// \sa getUserMode
  UserModeKind UserMode;

  /// Controls the mode of inter-procedural analysis.
  IPAKind IPAMode;

  /// Controls which C++ member functions will be considered for inlining.
  CXXInlineableMemberKind CXXMemberInliningMode;
  
  /// \sa includeTemporaryDtorsInCFG
  Optional<bool> IncludeTemporaryDtorsInCFG;
  
  /// \sa mayInlineCXXStandardLibrary
  Optional<bool> InlineCXXStandardLibrary;
  
  /// \sa mayInlineTemplateFunctions
  Optional<bool> InlineTemplateFunctions;

  /// \sa mayInlineCXXAllocator
  Optional<bool> InlineCXXAllocator;

  /// \sa mayInlineCXXContainerMethods
  Optional<bool> InlineCXXContainerMethods;

  /// \sa mayInlineCXXSharedPtrDtor
  Optional<bool> InlineCXXSharedPtrDtor;

  /// \sa mayInlineObjCMethod
  Optional<bool> ObjCInliningMode;

  // Cache of the "ipa-always-inline-size" setting.
  // \sa getAlwaysInlineSize
  Optional<unsigned> AlwaysInlineSize;

  /// \sa shouldSuppressNullReturnPaths
  Optional<bool> SuppressNullReturnPaths;

  // \sa getMaxInlinableSize
  Optional<unsigned> MaxInlinableSize;

  /// \sa shouldAvoidSuppressingNullArgumentPaths
  Optional<bool> AvoidSuppressingNullArgumentPaths;

  /// \sa shouldSuppressInlinedDefensiveChecks
  Optional<bool> SuppressInlinedDefensiveChecks;

  /// \sa shouldSuppressFromCXXStandardLibrary
  Optional<bool> SuppressFromCXXStandardLibrary;

  /// \sa reportIssuesInMainSourceFile
  Optional<bool> ReportIssuesInMainSourceFile;

  /// \sa StableReportFilename
  Optional<bool> StableReportFilename;

  /// \sa getGraphTrimInterval
  Optional<unsigned> GraphTrimInterval;

  /// \sa getMaxTimesInlineLarge
  Optional<unsigned> MaxTimesInlineLarge;

  /// \sa getMaxNodesPerTopLevelFunction
  Optional<unsigned> MaxNodesPerTopLevelFunction;

  /// A helper function that retrieves option for a given full-qualified
  /// checker name.
  /// Options for checkers can be specified via 'analyzer-config' command-line
  /// option.
  /// Example:
  /// @code-analyzer-config unix.Malloc:OptionName=CheckerOptionValue @endcode
  /// or @code-analyzer-config unix:OptionName=GroupOptionValue @endcode
  /// for groups of checkers.
  /// @param [in] CheckerName  Full-qualified checker name, like
  /// alpha.unix.StreamChecker.
  /// @param [in] OptionName  Name of the option to get.
  /// @param [in] Default  Default value if no option is specified.
  /// @param [in] SearchInParents If set to true and the searched option was not
  /// specified for the given checker the options for the parent packages will
  /// be searched as well. The inner packages take precedence over the outer
  /// ones.
  /// @retval CheckerOptionValue  An option for a checker if it was specified.
  /// @retval GroupOptionValue  An option for group if it was specified and no
  /// checker-specific options were found. The closer group to checker,
  /// the more priority it has. For example, @c coregroup.subgroup has more
  /// priority than @c coregroup for @c coregroup.subgroup.CheckerName checker.
  /// @retval Default  If nor checker option, nor group option was found.
  StringRef getCheckerOption(StringRef CheckerName, StringRef OptionName,
                             StringRef Default,
                             bool SearchInParents = false);

public:
  /// Interprets an option's string value as a boolean. The "true" string is
  /// interpreted as true and the "false" string is interpreted as false.
  ///
  /// If an option value is not provided, returns the given \p DefaultVal.
  /// @param [in] Name Name for option to retrieve.
  /// @param [in] DefaultVal Default value returned if no such option was
  /// specified.
  /// @param [in] C The optional checker parameter that can be used to restrict
  /// the search to the options of this particular checker (and its parents
  /// dependening on search mode).
  /// @param [in] SearchInParents If set to true and the searched option was not
  /// specified for the given checker the options for the parent packages will
  /// be searched as well. The inner packages take precedence over the outer
  /// ones.
  bool getBooleanOption(StringRef Name, bool DefaultVal,
                        const ento::CheckerBase *C = nullptr,
                        bool SearchInParents = false);

  /// Variant that accepts a Optional value to cache the result.
  ///
  /// @param [in,out] V Return value storage, returned if parameter contains
  /// an existing valid option, else it is used to store a return value
  /// @param [in] Name Name for option to retrieve.
  /// @param [in] DefaultVal Default value returned if no such option was
  /// specified.
  /// @param [in] C The optional checker parameter that can be used to restrict
  /// the search to the options of this particular checker (and its parents
  /// dependening on search mode).
  /// @param [in] SearchInParents If set to true and the searched option was not
  /// specified for the given checker the options for the parent packages will
  /// be searched as well. The inner packages take precedence over the outer
  /// ones.
  bool getBooleanOption(Optional<bool> &V, StringRef Name, bool DefaultVal,
                        const ento::CheckerBase *C  = nullptr,
                        bool SearchInParents = false);

  /// Interprets an option's string value as an integer value.
  ///
  /// If an option value is not provided, returns the given \p DefaultVal.
  /// @param [in] Name Name for option to retrieve.
  /// @param [in] DefaultVal Default value returned if no such option was
  /// specified.
  /// @param [in] C The optional checker parameter that can be used to restrict
  /// the search to the options of this particular checker (and its parents
  /// dependening on search mode).
  /// @param [in] SearchInParents If set to true and the searched option was not
  /// specified for the given checker the options for the parent packages will
  /// be searched as well. The inner packages take precedence over the outer
  /// ones.
  int getOptionAsInteger(StringRef Name, int DefaultVal,
                         const ento::CheckerBase *C = nullptr,
                         bool SearchInParents = false);

  /// Query an option's string value.
  ///
  /// If an option value is not provided, returns the given \p DefaultVal.
  /// @param [in] Name Name for option to retrieve.
  /// @param [in] DefaultVal Default value returned if no such option was
  /// specified.
  /// @param [in] C The optional checker parameter that can be used to restrict
  /// the search to the options of this particular checker (and its parents
  /// dependening on search mode).
  /// @param [in] SearchInParents If set to true and the searched option was not
  /// specified for the given checker the options for the parent packages will
  /// be searched as well. The inner packages take precedence over the outer
  /// ones.
  StringRef getOptionAsString(StringRef Name, StringRef DefaultVal,
                              const ento::CheckerBase *C = nullptr,
                              bool SearchInParents = false);

  /// \brief Retrieves and sets the UserMode. This is a high-level option,
  /// which is used to set other low-level options. It is not accessible
  /// outside of AnalyzerOptions.
  UserModeKind getUserMode();

  /// \brief Returns the inter-procedural analysis mode.
  IPAKind getIPAMode();

  /// Returns the option controlling which C++ member functions will be
  /// considered for inlining.
  ///
  /// This is controlled by the 'c++-inlining' config option.
  ///
  /// \sa CXXMemberInliningMode
  bool mayInlineCXXMemberFunction(CXXInlineableMemberKind K);

  /// Returns true if ObjectiveC inlining is enabled, false otherwise.
  bool mayInlineObjCMethod();

  /// Returns whether or not the destructors for C++ temporary objects should
  /// be included in the CFG.
  ///
  /// This is controlled by the 'cfg-temporary-dtors' config option, which
  /// accepts the values "true" and "false".
  bool includeTemporaryDtorsInCFG();

  /// Returns whether or not C++ standard library functions may be considered
  /// for inlining.
  ///
  /// This is controlled by the 'c++-stdlib-inlining' config option, which
  /// accepts the values "true" and "false".
  bool mayInlineCXXStandardLibrary();

  /// Returns whether or not templated functions may be considered for inlining.
  ///
  /// This is controlled by the 'c++-template-inlining' config option, which
  /// accepts the values "true" and "false".
  bool mayInlineTemplateFunctions();

  /// Returns whether or not allocator call may be considered for inlining.
  ///
  /// This is controlled by the 'c++-allocator-inlining' config option, which
  /// accepts the values "true" and "false".
  bool mayInlineCXXAllocator();

  /// Returns whether or not methods of C++ container objects may be considered
  /// for inlining.
  ///
  /// This is controlled by the 'c++-container-inlining' config option, which
  /// accepts the values "true" and "false".
  bool mayInlineCXXContainerMethods();

  /// Returns whether or not the destructor of C++ 'shared_ptr' may be
  /// considered for inlining.
  ///
  /// This covers std::shared_ptr, std::tr1::shared_ptr, and boost::shared_ptr,
  /// and indeed any destructor named "~shared_ptr".
  ///
  /// This is controlled by the 'c++-shared_ptr-inlining' config option, which
  /// accepts the values "true" and "false".
  bool mayInlineCXXSharedPtrDtor();

  /// Returns whether or not paths that go through null returns should be
  /// suppressed.
  ///
  /// This is a heuristic for avoiding bug reports with paths that go through
  /// inlined functions that are more defensive than their callers.
  ///
  /// This is controlled by the 'suppress-null-return-paths' config option,
  /// which accepts the values "true" and "false".
  bool shouldSuppressNullReturnPaths();

  /// Returns whether a bug report should \em not be suppressed if its path
  /// includes a call with a null argument, even if that call has a null return.
  ///
  /// This option has no effect when #shouldSuppressNullReturnPaths() is false.
  ///
  /// This is a counter-heuristic to avoid false negatives.
  ///
  /// This is controlled by the 'avoid-suppressing-null-argument-paths' config
  /// option, which accepts the values "true" and "false".
  bool shouldAvoidSuppressingNullArgumentPaths();

  /// Returns whether or not diagnostics containing inlined defensive NULL
  /// checks should be suppressed.
  ///
  /// This is controlled by the 'suppress-inlined-defensive-checks' config
  /// option, which accepts the values "true" and "false".
  bool shouldSuppressInlinedDefensiveChecks();

  /// Returns whether or not diagnostics reported within the C++ standard
  /// library should be suppressed.
  ///
  /// This is controlled by the 'suppress-c++-stdlib' config option,
  /// which accepts the values "true" and "false".
  bool shouldSuppressFromCXXStandardLibrary();

  /// Returns whether or not the diagnostic report should be always reported
  /// in the main source file and not the headers.
  ///
  /// This is controlled by the 'report-in-main-source-file' config option,
  /// which accepts the values "true" and "false".
  bool shouldReportIssuesInMainSourceFile();

  /// Returns whether or not the report filename should be random or not.
  ///
  /// This is controlled by the 'stable-report-filename' config option,
  /// which accepts the values "true" and "false". Default = false
  bool shouldWriteStableReportFilename();

  /// Returns whether irrelevant parts of a bug report path should be pruned
  /// out of the final output.
  ///
  /// This is controlled by the 'prune-paths' config option, which accepts the
  /// values "true" and "false".
  bool shouldPrunePaths();

  /// Returns true if 'static' initializers should be in conditional logic
  /// in the CFG.
  bool shouldConditionalizeStaticInitializers();

  // Returns the size of the functions (in basic blocks), which should be
  // considered to be small enough to always inline.
  //
  // This is controlled by "ipa-always-inline-size" analyzer-config option.
  unsigned getAlwaysInlineSize();

  // Returns the bound on the number of basic blocks in an inlined function
  // (50 by default).
  //
  // This is controlled by "-analyzer-config max-inlinable-size" option.
  unsigned getMaxInlinableSize();

  /// Returns true if the analyzer engine should synthesize fake bodies
  /// for well-known functions.
  bool shouldSynthesizeBodies();

  /// Returns how often nodes in the ExplodedGraph should be recycled to save
  /// memory.
  ///
  /// This is controlled by the 'graph-trim-interval' config option. To disable
  /// node reclamation, set the option to "0".
  unsigned getGraphTrimInterval();

  /// Returns the maximum times a large function could be inlined.
  ///
  /// This is controlled by the 'max-times-inline-large' config option.
  unsigned getMaxTimesInlineLarge();

  /// Returns the maximum number of nodes the analyzer can generate while
  /// exploring a top level function (for each exploded graph).
  /// 150000 is default; 0 means no limit.
  ///
  /// This is controlled by the 'max-nodes' config option.
  unsigned getMaxNodesPerTopLevelFunction();

public:
  AnalyzerOptions() :
    AnalysisStoreOpt(RegionStoreModel),
    AnalysisConstraintsOpt(RangeConstraintsModel),
    AnalysisDiagOpt(PD_HTML),
    AnalysisPurgeOpt(PurgeStmt),
    DisableAllChecks(0),
    ShowCheckerHelp(0),
    AnalyzeAll(0),
    AnalyzerDisplayProgress(0),
    AnalyzeNestedBlocks(0),
    eagerlyAssumeBinOpBifurcation(0),
    TrimGraph(0),
    visualizeExplodedGraphWithGraphViz(0),
    visualizeExplodedGraphWithUbiGraph(0),
    UnoptimizedCFG(0),
    PrintStats(0),
    NoRetryExhausted(0),
    // Cap the stack depth at 4 calls (5 stack frames, base + 4 calls).
    InlineMaxStackDepth(5),
    InliningMode(NoRedundancy),
    UserMode(UMK_NotSet),
    IPAMode(IPAK_NotSet),
    CXXMemberInliningMode() {}

};
  
typedef IntrusiveRefCntPtr<AnalyzerOptions> AnalyzerOptionsRef;
  
}

#endif
