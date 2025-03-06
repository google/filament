//===---  BugReporter.h - Generate PathDiagnostics --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines BugReporter, a utility class for generating
//  PathDiagnostics for analyses based on ProgramState.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CORE_BUGREPORTER_BUGREPORTER_H
#define LLVM_CLANG_STATICANALYZER_CORE_BUGREPORTER_BUGREPORTER_H

#include "clang/Basic/SourceLocation.h"
#include "clang/StaticAnalyzer/Core/AnalyzerOptions.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugReporterVisitor.h"
#include "clang/StaticAnalyzer/Core/BugReporter/PathDiagnostic.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/FoldingSet.h"
#include "llvm/ADT/ImmutableSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"

namespace clang {

class ASTContext;
class DiagnosticsEngine;
class Stmt;
class ParentMap;

namespace ento {

class PathDiagnostic;
class ExplodedNode;
class ExplodedGraph;
class BugReport;
class BugReporter;
class BugReporterContext;
class ExprEngine;
class BugType;

//===----------------------------------------------------------------------===//
// Interface for individual bug reports.
//===----------------------------------------------------------------------===//

/// This class provides an interface through which checkers can create
/// individual bug reports.
class BugReport : public llvm::ilist_node<BugReport> {
public:  
  class NodeResolver {
    virtual void anchor();
  public:
    virtual ~NodeResolver() {}
    virtual const ExplodedNode*
            getOriginalNode(const ExplodedNode *N) = 0;
  };

  typedef const SourceRange *ranges_iterator;
  typedef SmallVector<std::unique_ptr<BugReporterVisitor>, 8> VisitorList;
  typedef VisitorList::iterator visitor_iterator;
  typedef SmallVector<StringRef, 2> ExtraTextList;

protected:
  friend class BugReporter;
  friend class BugReportEquivClass;

  BugType& BT;
  const Decl *DeclWithIssue;
  std::string ShortDescription;
  std::string Description;
  PathDiagnosticLocation Location;
  PathDiagnosticLocation UniqueingLocation;
  const Decl *UniqueingDecl;
  
  const ExplodedNode *ErrorNode;
  SmallVector<SourceRange, 4> Ranges;
  ExtraTextList ExtraText;
  
  typedef llvm::DenseSet<SymbolRef> Symbols;
  typedef llvm::DenseSet<const MemRegion *> Regions;

  /// A (stack of) a set of symbols that are registered with this
  /// report as being "interesting", and thus used to help decide which
  /// diagnostics to include when constructing the final path diagnostic.
  /// The stack is largely used by BugReporter when generating PathDiagnostics
  /// for multiple PathDiagnosticConsumers.
  SmallVector<Symbols *, 2> interestingSymbols;

  /// A (stack of) set of regions that are registered with this report as being
  /// "interesting", and thus used to help decide which diagnostics
  /// to include when constructing the final path diagnostic.
  /// The stack is largely used by BugReporter when generating PathDiagnostics
  /// for multiple PathDiagnosticConsumers.
  SmallVector<Regions *, 2> interestingRegions;

  /// A set of location contexts that correspoind to call sites which should be
  /// considered "interesting".
  llvm::SmallSet<const LocationContext *, 2> InterestingLocationContexts;

  /// A set of custom visitors which generate "event" diagnostics at
  /// interesting points in the path.
  VisitorList Callbacks;

  /// Used for ensuring the visitors are only added once.
  llvm::FoldingSet<BugReporterVisitor> CallbacksSet;

  /// Used for clients to tell if the report's configuration has changed
  /// since the last time they checked.
  unsigned ConfigurationChangeToken;
  
  /// When set, this flag disables all callstack pruning from a diagnostic
  /// path.  This is useful for some reports that want maximum fidelty
  /// when reporting an issue.
  bool DoNotPrunePath;

  /// Used to track unique reasons why a bug report might be invalid.
  ///
  /// \sa markInvalid
  /// \sa removeInvalidation
  typedef std::pair<const void *, const void *> InvalidationRecord;

  /// If non-empty, this bug report is likely a false positive and should not be
  /// shown to the user.
  ///
  /// \sa markInvalid
  /// \sa removeInvalidation
  llvm::SmallSet<InvalidationRecord, 4> Invalidations;

private:
  // Used internally by BugReporter.
  Symbols &getInterestingSymbols();
  Regions &getInterestingRegions();

  void lazyInitializeInterestingSets();
  void pushInterestingSymbolsAndRegions();
  void popInterestingSymbolsAndRegions();

public:
  BugReport(BugType& bt, StringRef desc, const ExplodedNode *errornode)
    : BT(bt), DeclWithIssue(nullptr), Description(desc), ErrorNode(errornode),
      ConfigurationChangeToken(0), DoNotPrunePath(false) {}

  BugReport(BugType& bt, StringRef shortDesc, StringRef desc,
            const ExplodedNode *errornode)
    : BT(bt), DeclWithIssue(nullptr), ShortDescription(shortDesc),
      Description(desc), ErrorNode(errornode), ConfigurationChangeToken(0),
      DoNotPrunePath(false) {}

  BugReport(BugType &bt, StringRef desc, PathDiagnosticLocation l)
    : BT(bt), DeclWithIssue(nullptr), Description(desc), Location(l),
      ErrorNode(nullptr), ConfigurationChangeToken(0), DoNotPrunePath(false) {}

  /// \brief Create a BugReport with a custom uniqueing location.
  ///
  /// The reports that have the same report location, description, bug type, and
  /// ranges are uniqued - only one of the equivalent reports will be presented
  /// to the user. This method allows to rest the location which should be used
  /// for uniquing reports. For example, memory leaks checker, could set this to
  /// the allocation site, rather then the location where the bug is reported.
  BugReport(BugType& bt, StringRef desc, const ExplodedNode *errornode,
            PathDiagnosticLocation LocationToUnique, const Decl *DeclToUnique)
    : BT(bt), DeclWithIssue(nullptr), Description(desc),
      UniqueingLocation(LocationToUnique),
      UniqueingDecl(DeclToUnique),
      ErrorNode(errornode), ConfigurationChangeToken(0),
      DoNotPrunePath(false) {}

  virtual ~BugReport();

  const BugType& getBugType() const { return BT; }
  BugType& getBugType() { return BT; }

  const ExplodedNode *getErrorNode() const { return ErrorNode; }

  StringRef getDescription() const { return Description; }

  StringRef getShortDescription(bool UseFallback = true) const {
    if (ShortDescription.empty() && UseFallback)
      return Description;
    return ShortDescription;
  }

  /// Indicates whether or not any path pruning should take place
  /// when generating a PathDiagnostic from this BugReport.
  bool shouldPrunePath() const { return !DoNotPrunePath; }

  /// Disable all path pruning when generating a PathDiagnostic.
  void disablePathPruning() { DoNotPrunePath = true; }
  
  void markInteresting(SymbolRef sym);
  void markInteresting(const MemRegion *R);
  void markInteresting(SVal V);
  void markInteresting(const LocationContext *LC);
  
  bool isInteresting(SymbolRef sym);
  bool isInteresting(const MemRegion *R);
  bool isInteresting(SVal V);
  bool isInteresting(const LocationContext *LC);

  unsigned getConfigurationChangeToken() const {
    return ConfigurationChangeToken;
  }

  /// Returns whether or not this report should be considered valid.
  ///
  /// Invalid reports are those that have been classified as likely false
  /// positives after the fact.
  bool isValid() const {
    return Invalidations.empty();
  }

  /// Marks the current report as invalid, meaning that it is probably a false
  /// positive and should not be reported to the user.
  ///
  /// The \p Tag and \p Data arguments are intended to be opaque identifiers for
  /// this particular invalidation, where \p Tag represents the visitor
  /// responsible for invalidation, and \p Data represents the reason this
  /// visitor decided to invalidate the bug report.
  ///
  /// \sa removeInvalidation
  void markInvalid(const void *Tag, const void *Data) {
    Invalidations.insert(std::make_pair(Tag, Data));
  }

  /// Reverses the effects of a previous invalidation.
  ///
  /// \sa markInvalid
  void removeInvalidation(const void *Tag, const void *Data) {
    Invalidations.erase(std::make_pair(Tag, Data));
  }
  
  /// Return the canonical declaration, be it a method or class, where
  /// this issue semantically occurred.
  const Decl *getDeclWithIssue() const;
  
  /// Specifically set the Decl where an issue occurred.  This isn't necessary
  /// for BugReports that cover a path as it will be automatically inferred.
  void setDeclWithIssue(const Decl *declWithIssue) {
    DeclWithIssue = declWithIssue;
  }
  
  /// \brief This allows for addition of meta data to the diagnostic.
  ///
  /// Currently, only the HTMLDiagnosticClient knows how to display it. 
  void addExtraText(StringRef S) {
    ExtraText.push_back(S);
  }

  virtual const ExtraTextList &getExtraText() {
    return ExtraText;
  }

  /// \brief Return the "definitive" location of the reported bug.
  ///
  ///  While a bug can span an entire path, usually there is a specific
  ///  location that can be used to identify where the key issue occurred.
  ///  This location is used by clients rendering diagnostics.
  virtual PathDiagnosticLocation getLocation(const SourceManager &SM) const;

  /// \brief Get the location on which the report should be uniqued.
  PathDiagnosticLocation getUniqueingLocation() const {
    return UniqueingLocation;
  }
  
  /// \brief Get the declaration containing the uniqueing location.
  const Decl *getUniqueingDecl() const {
    return UniqueingDecl;
  }

  const Stmt *getStmt() const;

  /// \brief Add a range to a bug report.
  ///
  /// Ranges are used to highlight regions of interest in the source code.
  /// They should be at the same source code line as the BugReport location.
  /// By default, the source range of the statement corresponding to the error
  /// node will be used; add a single invalid range to specify absence of
  /// ranges.
  void addRange(SourceRange R) {
    assert((R.isValid() || Ranges.empty()) && "Invalid range can only be used "
                           "to specify that the report does not have a range.");
    Ranges.push_back(R);
  }

  /// \brief Get the SourceRanges associated with the report.
  virtual llvm::iterator_range<ranges_iterator> getRanges();

  /// \brief Add custom or predefined bug report visitors to this report.
  ///
  /// The visitors should be used when the default trace is not sufficient.
  /// For example, they allow constructing a more elaborate trace.
  /// \sa registerConditionVisitor(), registerTrackNullOrUndefValue(),
  /// registerFindLastStore(), registerNilReceiverVisitor(), and
  /// registerVarDeclsLastStore().
  void addVisitor(std::unique_ptr<BugReporterVisitor> visitor);

  /// Iterators through the custom diagnostic visitors.
  visitor_iterator visitor_begin() { return Callbacks.begin(); }
  visitor_iterator visitor_end() { return Callbacks.end(); }

  /// Profile to identify equivalent bug reports for error report coalescing.
  /// Reports are uniqued to ensure that we do not emit multiple diagnostics
  /// for each bug.
  virtual void Profile(llvm::FoldingSetNodeID& hash) const;
};

} // end ento namespace
} // end clang namespace

namespace llvm {
  template<> struct ilist_traits<clang::ento::BugReport>
    : public ilist_default_traits<clang::ento::BugReport> {
  // HLSL Change Starts
  // Temporarily disable "downcast of address" UBSAN runtime error
  // https://github.com/microsoft/DirectXShaderCompiler/issues/6446
#ifdef __has_feature
#if __has_feature(undefined_behavior_sanitizer)
    __attribute__((no_sanitize("undefined")))
#endif // __has_feature(address_sanitizer)
#endif // defined(__has_feature)
         // HLSL Change Ends
    clang::ento::BugReport *
    createSentinel() const {
      return static_cast<clang::ento::BugReport *>(&Sentinel);
    }
    void destroySentinel(clang::ento::BugReport *) const {}

    clang::ento::BugReport *provideInitialHead() const {
      return createSentinel();
    }
    clang::ento::BugReport *ensureHead(clang::ento::BugReport *) const {
      return createSentinel();
    }
  private:
    mutable ilist_half_node<clang::ento::BugReport> Sentinel;
  };
}

namespace clang {
namespace ento {

//===----------------------------------------------------------------------===//
// BugTypes (collections of related reports).
//===----------------------------------------------------------------------===//

class BugReportEquivClass : public llvm::FoldingSetNode {
  /// List of *owned* BugReport objects.
  llvm::ilist<BugReport> Reports;

  friend class BugReporter;
  void AddReport(std::unique_ptr<BugReport> R) {
    Reports.push_back(R.release());
  }

public:
  BugReportEquivClass(std::unique_ptr<BugReport> R) { AddReport(std::move(R)); }
  ~BugReportEquivClass();

  void Profile(llvm::FoldingSetNodeID& ID) const {
    assert(!Reports.empty());
    Reports.front().Profile(ID);
  }

  typedef llvm::ilist<BugReport>::iterator iterator;
  typedef llvm::ilist<BugReport>::const_iterator const_iterator;

  iterator begin() { return Reports.begin(); }
  iterator end() { return Reports.end(); }

  const_iterator begin() const { return Reports.begin(); }
  const_iterator end() const { return Reports.end(); }
};

//===----------------------------------------------------------------------===//
// BugReporter and friends.
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

class BugReporterData {
public:
  virtual ~BugReporterData();
  virtual DiagnosticsEngine& getDiagnostic() = 0;
  virtual ArrayRef<PathDiagnosticConsumer*> getPathDiagnosticConsumers() = 0;
  virtual ASTContext &getASTContext() = 0;
  virtual SourceManager& getSourceManager() = 0;
  virtual AnalyzerOptions& getAnalyzerOptions() = 0;
};

/// BugReporter is a utility class for generating PathDiagnostics for analysis.
/// It collects the BugReports and BugTypes and knows how to generate
/// and flush the corresponding diagnostics.
class BugReporter {
public:
  enum Kind { BaseBRKind, GRBugReporterKind };

private:
  typedef llvm::ImmutableSet<BugType*> BugTypesTy;
  BugTypesTy::Factory F;
  BugTypesTy BugTypes;

  const Kind kind;
  BugReporterData& D;

  /// Generate and flush the diagnostics for the given bug report.
  void FlushReport(BugReportEquivClass& EQ);

  /// Generate and flush the diagnostics for the given bug report
  /// and PathDiagnosticConsumer.
  void FlushReport(BugReport *exampleReport,
                   PathDiagnosticConsumer &PD,
                   ArrayRef<BugReport*> BugReports);

  /// The set of bug reports tracked by the BugReporter.
  llvm::FoldingSet<BugReportEquivClass> EQClasses;
  /// A vector of BugReports for tracking the allocated pointers and cleanup.
  std::vector<BugReportEquivClass *> EQClassesVector;

protected:
  BugReporter(BugReporterData& d, Kind k) : BugTypes(F.getEmptySet()), kind(k),
                                            D(d) {}

public:
  BugReporter(BugReporterData& d) : BugTypes(F.getEmptySet()), kind(BaseBRKind),
                                    D(d) {}
  virtual ~BugReporter();

  /// \brief Generate and flush diagnostics for all bug reports.
  void FlushReports();

  Kind getKind() const { return kind; }

  DiagnosticsEngine& getDiagnostic() {
    return D.getDiagnostic();
  }

  ArrayRef<PathDiagnosticConsumer*> getPathDiagnosticConsumers() {
    return D.getPathDiagnosticConsumers();
  }

  /// \brief Iterator over the set of BugTypes tracked by the BugReporter.
  typedef BugTypesTy::iterator iterator;
  iterator begin() { return BugTypes.begin(); }
  iterator end() { return BugTypes.end(); }

  /// \brief Iterator over the set of BugReports tracked by the BugReporter.
  typedef llvm::FoldingSet<BugReportEquivClass>::iterator EQClasses_iterator;
  EQClasses_iterator EQClasses_begin() { return EQClasses.begin(); }
  EQClasses_iterator EQClasses_end() { return EQClasses.end(); }

  ASTContext &getContext() { return D.getASTContext(); }

  SourceManager& getSourceManager() { return D.getSourceManager(); }

  AnalyzerOptions& getAnalyzerOptions() { return D.getAnalyzerOptions(); }

  virtual bool generatePathDiagnostic(PathDiagnostic& pathDiagnostic,
                                      PathDiagnosticConsumer &PC,
                                      ArrayRef<BugReport *> &bugReports) {
    return true;
  }

  bool RemoveUnneededCalls(PathPieces &pieces, BugReport *R);

  void Register(BugType *BT);

  /// \brief Add the given report to the set of reports tracked by BugReporter.
  ///
  /// The reports are usually generated by the checkers. Further, they are
  /// folded based on the profile value, which is done to coalesce similar
  /// reports.
  void emitReport(std::unique_ptr<BugReport> R);

  void EmitBasicReport(const Decl *DeclWithIssue, const CheckerBase *Checker,
                       StringRef BugName, StringRef BugCategory,
                       StringRef BugStr, PathDiagnosticLocation Loc,
                       ArrayRef<SourceRange> Ranges = None);

  void EmitBasicReport(const Decl *DeclWithIssue, CheckName CheckName,
                       StringRef BugName, StringRef BugCategory,
                       StringRef BugStr, PathDiagnosticLocation Loc,
                       ArrayRef<SourceRange> Ranges = None);

private:
  llvm::StringMap<BugType *> StrBugTypes;

  /// \brief Returns a BugType that is associated with the given name and
  /// category.
  BugType *getBugTypeForName(CheckName CheckName, StringRef name,
                             StringRef category);
};

// FIXME: Get rid of GRBugReporter.  It's the wrong abstraction.
class GRBugReporter : public BugReporter {
  ExprEngine& Eng;
public:
  GRBugReporter(BugReporterData& d, ExprEngine& eng)
    : BugReporter(d, GRBugReporterKind), Eng(eng) {}

  ~GRBugReporter() override;

  /// getEngine - Return the analysis engine used to analyze a given
  ///  function or method.
  ExprEngine &getEngine() { return Eng; }

  /// getGraph - Get the exploded graph created by the analysis engine
  ///  for the analyzed method or function.
  ExplodedGraph &getGraph();

  /// getStateManager - Return the state manager used by the analysis
  ///  engine.
  ProgramStateManager &getStateManager();

  /// Generates a path corresponding to one of the given bug reports.
  ///
  /// Which report is used for path generation is not specified. The
  /// bug reporter will try to pick the shortest path, but this is not
  /// guaranteed.
  ///
  /// \return True if the report was valid and a path was generated,
  ///         false if the reports should be considered invalid.
  bool generatePathDiagnostic(PathDiagnostic &PD, PathDiagnosticConsumer &PC,
                              ArrayRef<BugReport*> &bugReports) override;

  /// classof - Used by isa<>, cast<>, and dyn_cast<>.
  static bool classof(const BugReporter* R) {
    return R->getKind() == GRBugReporterKind;
  }
};

class BugReporterContext {
  virtual void anchor();
  GRBugReporter &BR;
public:
  BugReporterContext(GRBugReporter& br) : BR(br) {}

  virtual ~BugReporterContext() {}

  GRBugReporter& getBugReporter() { return BR; }

  ExplodedGraph &getGraph() { return BR.getGraph(); }

  ProgramStateManager& getStateManager() {
    return BR.getStateManager();
  }

  SValBuilder& getSValBuilder() {
    return getStateManager().getSValBuilder();
  }

  ASTContext &getASTContext() {
    return BR.getContext();
  }

  SourceManager& getSourceManager() {
    return BR.getSourceManager();
  }

  virtual BugReport::NodeResolver& getNodeResolver() = 0;
};

} // end GR namespace

} // end clang namespace

#endif
