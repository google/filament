//== Environment.h - Map from Stmt* to Locations/Values ---------*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defined the Environment and EnvironmentManager classes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_ENVIRONMENT_H
#define LLVM_CLANG_STATICANALYZER_CORE_PATHSENSITIVE_ENVIRONMENT_H

#include "clang/Analysis/AnalysisContext.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/SVals.h"
#include "llvm/ADT/ImmutableMap.h"

namespace clang {

class LiveVariables;

namespace ento {

class EnvironmentManager;
class SValBuilder;

/// An entry in the environment consists of a Stmt and an LocationContext.
/// This allows the environment to manage context-sensitive bindings,
/// which is essentially for modeling recursive function analysis, among
/// other things.
class EnvironmentEntry : public std::pair<const Stmt*,
                                          const StackFrameContext *> {
public:
  EnvironmentEntry(const Stmt *s, const LocationContext *L);

  const Stmt *getStmt() const { return first; }
  const LocationContext *getLocationContext() const { return second; }
  
  /// Profile an EnvironmentEntry for inclusion in a FoldingSet.
  static void Profile(llvm::FoldingSetNodeID &ID,
                      const EnvironmentEntry &E) {
    ID.AddPointer(E.getStmt());
    ID.AddPointer(E.getLocationContext());
  }
  
  void Profile(llvm::FoldingSetNodeID &ID) const {
    Profile(ID, *this);
  }
};

/// An immutable map from EnvironemntEntries to SVals.
class Environment {
private:
  friend class EnvironmentManager;

  // Type definitions.
  typedef llvm::ImmutableMap<EnvironmentEntry, SVal> BindingsTy;

  // Data.
  BindingsTy ExprBindings;

  Environment(BindingsTy eb)
    : ExprBindings(eb) {}

  SVal lookupExpr(const EnvironmentEntry &E) const;

public:
  typedef BindingsTy::iterator iterator;
  iterator begin() const { return ExprBindings.begin(); }
  iterator end() const { return ExprBindings.end(); }

  /// Fetches the current binding of the expression in the
  /// Environment.
  SVal getSVal(const EnvironmentEntry &E, SValBuilder &svalBuilder) const;

  /// Profile - Profile the contents of an Environment object for use
  ///  in a FoldingSet.
  static void Profile(llvm::FoldingSetNodeID& ID, const Environment* env) {
    env->ExprBindings.Profile(ID);
  }

  /// Profile - Used to profile the contents of this object for inclusion
  ///  in a FoldingSet.
  void Profile(llvm::FoldingSetNodeID& ID) const {
    Profile(ID, this);
  }

  bool operator==(const Environment& RHS) const {
    return ExprBindings == RHS.ExprBindings;
  }
  
  void print(raw_ostream &Out, const char *NL, const char *Sep) const;
  
private:
  void printAux(raw_ostream &Out, bool printLocations,
                const char *NL, const char *Sep) const;
};

class EnvironmentManager {
private:
  typedef Environment::BindingsTy::Factory FactoryTy;
  FactoryTy F;

public:
  EnvironmentManager(llvm::BumpPtrAllocator& Allocator) : F(Allocator) {}

  Environment getInitialEnvironment() {
    return Environment(F.getEmptyMap());
  }

  /// Bind a symbolic value to the given environment entry.
  Environment bindExpr(Environment Env, const EnvironmentEntry &E, SVal V,
                       bool Invalidate);

  Environment removeDeadBindings(Environment Env,
                                 SymbolReaper &SymReaper,
                                 ProgramStateRef state);
};

} // end GR namespace

} // end clang namespace

#endif
