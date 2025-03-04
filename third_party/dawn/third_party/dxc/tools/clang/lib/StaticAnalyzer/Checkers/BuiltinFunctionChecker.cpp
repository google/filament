//=== BuiltinFunctionChecker.cpp --------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This checker evaluates clang builtin functions.
//
//===----------------------------------------------------------------------===//

#include "ClangSACheckers.h"
#include "clang/Basic/Builtins.h"
#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/CheckerManager.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace clang;
using namespace ento;

namespace {

class BuiltinFunctionChecker : public Checker<eval::Call> {
public:
  bool evalCall(const CallExpr *CE, CheckerContext &C) const;
};

}

bool BuiltinFunctionChecker::evalCall(const CallExpr *CE,
                                      CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  const FunctionDecl *FD = C.getCalleeDecl(CE);
  const LocationContext *LCtx = C.getLocationContext();
  if (!FD)
    return false;

  switch (FD->getBuiltinID()) {
  default:
    return false;

  case Builtin::BI__builtin_expect:
  case Builtin::BI__builtin_assume_aligned:
  case Builtin::BI__builtin_addressof: {
    // For __builtin_expect and __builtin_assume_aligned, just return the value
    // of the subexpression.
    // __builtin_addressof is going from a reference to a pointer, but those
    // are represented the same way in the analyzer.
    assert (CE->arg_begin() != CE->arg_end());
    SVal X = state->getSVal(*(CE->arg_begin()), LCtx);
    C.addTransition(state->BindExpr(CE, LCtx, X));
    return true;
  }

  case Builtin::BI__builtin_alloca: {
    // FIXME: Refactor into StoreManager itself?
    MemRegionManager& RM = C.getStoreManager().getRegionManager();
    const AllocaRegion* R =
      RM.getAllocaRegion(CE, C.blockCount(), C.getLocationContext());

    // Set the extent of the region in bytes. This enables us to use the
    // SVal of the argument directly. If we save the extent in bits, we
    // cannot represent values like symbol*8.
    DefinedOrUnknownSVal Size =
        state->getSVal(*(CE->arg_begin()), LCtx).castAs<DefinedOrUnknownSVal>();

    SValBuilder& svalBuilder = C.getSValBuilder();
    DefinedOrUnknownSVal Extent = R->getExtent(svalBuilder);
    DefinedOrUnknownSVal extentMatchesSizeArg =
      svalBuilder.evalEQ(state, Extent, Size);
    state = state->assume(extentMatchesSizeArg, true);
    assert(state && "The region should not have any previous constraints");

    C.addTransition(state->BindExpr(CE, LCtx, loc::MemRegionVal(R)));
    return true;
  }

  case Builtin::BI__builtin_object_size: {
    // This must be resolvable at compile time, so we defer to the constant
    // evaluator for a value.
    SVal V = UnknownVal();
    llvm::APSInt Result;
    if (CE->EvaluateAsInt(Result, C.getASTContext(), Expr::SE_NoSideEffects)) {
      // Make sure the result has the correct type.
      SValBuilder &SVB = C.getSValBuilder();
      BasicValueFactory &BVF = SVB.getBasicValueFactory();
      BVF.getAPSIntType(CE->getType()).apply(Result);
      V = SVB.makeIntVal(Result);
    }

    C.addTransition(state->BindExpr(CE, LCtx, V));
    return true;
  }
  }
}

void ento::registerBuiltinFunctionChecker(CheckerManager &mgr) {
  mgr.registerChecker<BuiltinFunctionChecker>();
}
