//== ConstraintManager.h - Constraints on symbolic values.-------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defined the interface to manage constraints on symbolic values.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_CONSTRAINTMANAGER_H
#define LLVM_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_CONSTRAINTMANAGER_H

#include "clang/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/SymbolManager.h"
#include "llvm/Support/SaveAndRestore.h"

namespace llvm {
class APSInt;
}

namespace clang {
namespace ento {

class SubEngine;

class ConditionTruthVal {
  Optional<bool> Val;
public:
  /// Construct a ConditionTruthVal indicating the constraint is constrained
  /// to either true or false, depending on the boolean value provided.
  ConditionTruthVal(bool constraint) : Val(constraint) {}

  /// Construct a ConstraintVal indicating the constraint is underconstrained.
  ConditionTruthVal() {}
  
  /// Return true if the constraint is perfectly constrained to 'true'.
  bool isConstrainedTrue() const {
    return Val.hasValue() && Val.getValue();
  }

  /// Return true if the constraint is perfectly constrained to 'false'.
  bool isConstrainedFalse() const {
    return Val.hasValue() && !Val.getValue();
  }

  /// Return true if the constrained is perfectly constrained.
  bool isConstrained() const {
    return Val.hasValue();
  }

  /// Return true if the constrained is underconstrained and we do not know
  /// if the constraint is true of value.
  bool isUnderconstrained() const {
    return !Val.hasValue();
  }
};
  
class ConstraintManager {
public:
  ConstraintManager() : NotifyAssumeClients(true) {}
  
  virtual ~ConstraintManager();
  virtual ProgramStateRef assume(ProgramStateRef state,
                                 DefinedSVal Cond,
                                 bool Assumption) = 0;

  typedef std::pair<ProgramStateRef, ProgramStateRef> ProgramStatePair;

  /// Returns a pair of states (StTrue, StFalse) where the given condition is
  /// assumed to be true or false, respectively.
  ProgramStatePair assumeDual(ProgramStateRef State, DefinedSVal Cond) {
    ProgramStateRef StTrue = assume(State, Cond, true);

    // If StTrue is infeasible, asserting the falseness of Cond is unnecessary
    // because the existing constraints already establish this.
    if (!StTrue) {
#ifndef __OPTIMIZE__
      // This check is expensive and should be disabled even in Release+Asserts
      // builds.
      // FIXME: __OPTIMIZE__ is a GNU extension that Clang implements but MSVC
      // does not. Is there a good equivalent there?
      assert(assume(State, Cond, false) && "System is over constrained.");
#endif
      return ProgramStatePair((ProgramStateRef)nullptr, State);
    }

    ProgramStateRef StFalse = assume(State, Cond, false);
    if (!StFalse) {
      // We are careful to return the original state, /not/ StTrue,
      // because we want to avoid having callers generate a new node
      // in the ExplodedGraph.
      return ProgramStatePair(State, (ProgramStateRef)nullptr);
    }

    return ProgramStatePair(StTrue, StFalse);
  }

  /// \brief If a symbol is perfectly constrained to a constant, attempt
  /// to return the concrete value.
  ///
  /// Note that a ConstraintManager is not obligated to return a concretized
  /// value for a symbol, even if it is perfectly constrained.
  virtual const llvm::APSInt* getSymVal(ProgramStateRef state,
                                        SymbolRef sym) const {
    return nullptr;
  }

  virtual ProgramStateRef removeDeadBindings(ProgramStateRef state,
                                                 SymbolReaper& SymReaper) = 0;

  virtual void print(ProgramStateRef state,
                     raw_ostream &Out,
                     const char* nl,
                     const char *sep) = 0;

  virtual void EndPath(ProgramStateRef state) {}
  
  /// Convenience method to query the state to see if a symbol is null or
  /// not null, or if neither assumption can be made.
  ConditionTruthVal isNull(ProgramStateRef State, SymbolRef Sym) {
    SaveAndRestore<bool> DisableNotify(NotifyAssumeClients, false);

    return checkNull(State, Sym);
  }

protected:
  /// A flag to indicate that clients should be notified of assumptions.
  /// By default this is the case, but sometimes this needs to be restricted
  /// to avoid infinite recursions within the ConstraintManager.
  ///
  /// Note that this flag allows the ConstraintManager to be re-entrant,
  /// but not thread-safe.
  bool NotifyAssumeClients;

  /// canReasonAbout - Not all ConstraintManagers can accurately reason about
  ///  all SVal values.  This method returns true if the ConstraintManager can
  ///  reasonably handle a given SVal value.  This is typically queried by
  ///  ExprEngine to determine if the value should be replaced with a
  ///  conjured symbolic value in order to recover some precision.
  virtual bool canReasonAbout(SVal X) const = 0;

  /// Returns whether or not a symbol is known to be null ("true"), known to be
  /// non-null ("false"), or may be either ("underconstrained"). 
  virtual ConditionTruthVal checkNull(ProgramStateRef State, SymbolRef Sym);
};

std::unique_ptr<ConstraintManager>
CreateRangeConstraintManager(ProgramStateManager &statemgr,
                             SubEngine *subengine);

} // end GR namespace

} // end clang namespace

#endif
