//= CheckerDocumentation.cpp - Documentation checker ---------------*- C++ -*-//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This checker lists all the checker callbacks and provides documentation for
// checker writers.
//
//===----------------------------------------------------------------------===//

#include "ClangSACheckers.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/ProgramStateTrait.h"

using namespace clang;
using namespace ento;

// All checkers should be placed into anonymous namespace.
// We place the CheckerDocumentation inside ento namespace to make the
// it visible in doxygen.
namespace clang {
namespace ento {

/// This checker documents the callback functions checkers can use to implement
/// the custom handling of the specific events during path exploration as well
/// as reporting bugs. Most of the callbacks are targeted at path-sensitive
/// checking.
///
/// \sa CheckerContext
class CheckerDocumentation : public Checker< check::PreStmt<ReturnStmt>,
                                       check::PostStmt<DeclStmt>,
                                       check::PreObjCMessage,
                                       check::PostObjCMessage,
                                       check::PreCall,
                                       check::PostCall,
                                       check::BranchCondition,
                                       check::Location,
                                       check::Bind,
                                       check::DeadSymbols,
                                       check::EndFunction,
                                       check::EndAnalysis,
                                       check::EndOfTranslationUnit,
                                       eval::Call,
                                       eval::Assume,
                                       check::LiveSymbols,
                                       check::RegionChanges,
                                       check::PointerEscape,
                                       check::ConstPointerEscape,
                                       check::Event<ImplicitNullDerefEvent>,
                                       check::ASTDecl<FunctionDecl> > {
public:

  /// \brief Pre-visit the Statement.
  ///
  /// The method will be called before the analyzer core processes the
  /// statement. The notification is performed for every explored CFGElement,
  /// which does not include the control flow statements such as IfStmt. The
  /// callback can be specialized to be called with any subclass of Stmt.
  ///
  /// See checkBranchCondition() callback for performing custom processing of
  /// the branching statements.
  ///
  /// check::PreStmt<ReturnStmt>
  void checkPreStmt(const ReturnStmt *DS, CheckerContext &C) const {}

  /// \brief Post-visit the Statement.
  ///
  /// The method will be called after the analyzer core processes the
  /// statement. The notification is performed for every explored CFGElement,
  /// which does not include the control flow statements such as IfStmt. The
  /// callback can be specialized to be called with any subclass of Stmt.
  ///
  /// check::PostStmt<DeclStmt>
  void checkPostStmt(const DeclStmt *DS, CheckerContext &C) const;

  /// \brief Pre-visit the Objective C message.
  ///
  /// This will be called before the analyzer core processes the method call.
  /// This is called for any action which produces an Objective-C message send,
  /// including explicit message syntax and property access.
  ///
  /// check::PreObjCMessage
  void checkPreObjCMessage(const ObjCMethodCall &M, CheckerContext &C) const {}

  /// \brief Post-visit the Objective C message.
  /// \sa checkPreObjCMessage()
  ///
  /// check::PostObjCMessage
  void checkPostObjCMessage(const ObjCMethodCall &M, CheckerContext &C) const {}

  /// \brief Pre-visit an abstract "call" event.
  ///
  /// This is used for checkers that want to check arguments or attributed
  /// behavior for functions and methods no matter how they are being invoked.
  ///
  /// Note that this includes ALL cross-body invocations, so if you want to
  /// limit your checks to, say, function calls, you should test for that at the
  /// beginning of your callback function.
  ///
  /// check::PreCall
  void checkPreCall(const CallEvent &Call, CheckerContext &C) const {}

  /// \brief Post-visit an abstract "call" event.
  /// \sa checkPreObjCMessage()
  ///
  /// check::PostCall
  void checkPostCall(const CallEvent &Call, CheckerContext &C) const {}

  /// \brief Pre-visit of the condition statement of a branch (such as IfStmt).
  void checkBranchCondition(const Stmt *Condition, CheckerContext &Ctx) const {}

  /// \brief Called on a load from and a store to a location.
  ///
  /// The method will be called each time a location (pointer) value is
  /// accessed.
  /// \param Loc    The value of the location (pointer).
  /// \param IsLoad The flag specifying if the location is a store or a load.
  /// \param S      The load is performed while processing the statement.
  ///
  /// check::Location
  void checkLocation(SVal Loc, bool IsLoad, const Stmt *S,
                     CheckerContext &) const {}

  /// \brief Called on binding of a value to a location.
  ///
  /// \param Loc The value of the location (pointer).
  /// \param Val The value which will be stored at the location Loc.
  /// \param S   The bind is performed while processing the statement S.
  ///
  /// check::Bind
  void checkBind(SVal Loc, SVal Val, const Stmt *S, CheckerContext &) const {}


  /// \brief Called whenever a symbol becomes dead.
  ///
  /// This callback should be used by the checkers to aggressively clean
  /// up/reduce the checker state, which is important for reducing the overall
  /// memory usage. Specifically, if a checker keeps symbol specific information
  /// in the sate, it can and should be dropped after the symbol becomes dead.
  /// In addition, reporting a bug as soon as the checker becomes dead leads to
  /// more precise diagnostics. (For example, one should report that a malloced
  /// variable is not freed right after it goes out of scope.)
  ///
  /// \param SR The SymbolReaper object can be queried to determine which
  ///           symbols are dead.
  ///
  /// check::DeadSymbols
  void checkDeadSymbols(SymbolReaper &SR, CheckerContext &C) const {}

  /// \brief Called when the analyzer core reaches the end of a
  /// function being analyzed.
  ///
  /// check::EndFunction
  void checkEndFunction(CheckerContext &Ctx) const {}

  /// \brief Called after all the paths in the ExplodedGraph reach end of path
  /// - the symbolic execution graph is fully explored.
  ///
  /// This callback should be used in cases when a checker needs to have a
  /// global view of the information generated on all paths. For example, to
  /// compare execution summary/result several paths.
  /// See IdempotentOperationChecker for a usage example.
  ///
  /// check::EndAnalysis
  void checkEndAnalysis(ExplodedGraph &G,
                        BugReporter &BR,
                        ExprEngine &Eng) const {}

  /// \brief Called after analysis of a TranslationUnit is complete.
  ///
  /// check::EndOfTranslationUnit
  void checkEndOfTranslationUnit(const TranslationUnitDecl *TU,
                                 AnalysisManager &Mgr,
                                 BugReporter &BR) const {}


  /// \brief Evaluates function call.
  ///
  /// The analysis core threats all function calls in the same way. However, some
  /// functions have special meaning, which should be reflected in the program
  /// state. This callback allows a checker to provide domain specific knowledge
  /// about the particular functions it knows about.
  ///
  /// \returns true if the call has been successfully evaluated
  /// and false otherwise. Note, that only one checker can evaluate a call. If
  /// more than one checker claims that they can evaluate the same call the
  /// first one wins.
  ///
  /// eval::Call
  bool evalCall(const CallExpr *CE, CheckerContext &C) const { return true; }

  /// \brief Handles assumptions on symbolic values.
  ///
  /// This method is called when a symbolic expression is assumed to be true or
  /// false. For example, the assumptions are performed when evaluating a
  /// condition at a branch. The callback allows checkers track the assumptions
  /// performed on the symbols of interest and change the state accordingly.
  ///
  /// eval::Assume
  ProgramStateRef evalAssume(ProgramStateRef State,
                                 SVal Cond,
                                 bool Assumption) const { return State; }

  /// Allows modifying SymbolReaper object. For example, checkers can explicitly
  /// register symbols of interest as live. These symbols will not be marked
  /// dead and removed.
  ///
  /// check::LiveSymbols
  void checkLiveSymbols(ProgramStateRef State, SymbolReaper &SR) const {}

  /// \brief Called to determine if the checker currently needs to know if when
  /// contents of any regions change.
  ///
  /// Since it is not necessarily cheap to compute which regions are being
  /// changed, this allows the analyzer core to skip the more expensive
  /// #checkRegionChanges when no checkers are tracking any state.
  bool wantsRegionChangeUpdate(ProgramStateRef St) const { return true; }
  
  /// \brief Called when the contents of one or more regions change.
  ///
  /// This can occur in many different ways: an explicit bind, a blanket
  /// invalidation of the region contents, or by passing a region to a function
  /// call whose behavior the analyzer cannot model perfectly.
  ///
  /// \param State The current program state.
  /// \param Invalidated A set of all symbols potentially touched by the change.
  /// \param ExplicitRegions The regions explicitly requested for invalidation.
  ///        For a function call, this would be the arguments. For a bind, this
  ///        would be the region being bound to.
  /// \param Regions The transitive closure of regions accessible from,
  ///        \p ExplicitRegions, i.e. all regions that may have been touched
  ///        by this change. For a simple bind, this list will be the same as
  ///        \p ExplicitRegions, since a bind does not affect the contents of
  ///        anything accessible through the base region.
  /// \param Call The opaque call triggering this invalidation. Will be 0 if the
  ///        change was not triggered by a call.
  ///
  /// Note that this callback will not be invoked unless
  /// #wantsRegionChangeUpdate returns \c true.
  ///
  /// check::RegionChanges
  ProgramStateRef 
    checkRegionChanges(ProgramStateRef State,
                       const InvalidatedSymbols *Invalidated,
                       ArrayRef<const MemRegion *> ExplicitRegions,
                       ArrayRef<const MemRegion *> Regions,
                       const CallEvent *Call) const {
    return State;
  }

  /// \brief Called when pointers escape.
  ///
  /// This notifies the checkers about pointer escape, which occurs whenever
  /// the analyzer cannot track the symbol any more. For example, as a
  /// result of assigning a pointer into a global or when it's passed to a 
  /// function call the analyzer cannot model.
  /// 
  /// \param State The state at the point of escape.
  /// \param Escaped The list of escaped symbols.
  /// \param Call The corresponding CallEvent, if the symbols escape as 
  /// parameters to the given call.
  /// \param Kind How the symbols have escaped.
  /// \returns Checkers can modify the state by returning a new state.
  ProgramStateRef checkPointerEscape(ProgramStateRef State,
                                     const InvalidatedSymbols &Escaped,
                                     const CallEvent *Call,
                                     PointerEscapeKind Kind) const {
    return State;
  }

  /// \brief Called when const pointers escape.
  ///
  /// Note: in most cases checkPointerEscape callback is sufficient.
  /// \sa checkPointerEscape
  ProgramStateRef checkConstPointerEscape(ProgramStateRef State,
                                     const InvalidatedSymbols &Escaped,
                                     const CallEvent *Call,
                                     PointerEscapeKind Kind) const {
    return State;
  }
                                         
  /// check::Event<ImplicitNullDerefEvent>
  void checkEvent(ImplicitNullDerefEvent Event) const {}

  /// \brief Check every declaration in the AST.
  ///
  /// An AST traversal callback, which should only be used when the checker is
  /// not path sensitive. It will be called for every Declaration in the AST and
  /// can be specialized to only be called on subclasses of Decl, for example,
  /// FunctionDecl.
  ///
  /// check::ASTDecl<FunctionDecl>
  void checkASTDecl(const FunctionDecl *D,
                    AnalysisManager &Mgr,
                    BugReporter &BR) const {}

};

void CheckerDocumentation::checkPostStmt(const DeclStmt *DS,
                                         CheckerContext &C) const {
  return;
}

} // end namespace ento
} // end namespace clang
